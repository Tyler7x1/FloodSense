require('dotenv').config();
const { connectToMongo } = require('./connect');
const { fetchData, pushData, getAllData } = require('./controllers/dataController');
const Data = require('./models/dataModel');
const express = require('express');
const app = express();
const http = require('http');
const cors = require('cors');
const server = http.createServer(app);
const socketIo = require('socket.io');
const io = socketIo(server);

const uri = process.env.MONGO_URI
const port = process.env.PORT;

app.use(cors());  // Enable CORS for all routes

app.set('view engine', 'ejs'); // Set EJS as the view engine
app.set('views', './views');   // Define the views folder

// Connect to MongoDB
connectToMongo(uri);

app.get('/', fetchData);
app.get('/data', getAllData);
io.on('connection', async (socket) => {
    console.log('A user connected');
    try {
      // Fetch the most recent data from the database and send it to the connected client
      const latestData = await Data.findOne().sort({ timestamp: -1 });
      if (latestData) {
        socket.emit('newData', latestData); // Emit data to client
      }
    } catch (err) {
      console.error("Error fetching latest data:", err);
    }
    // Listen for disconnection events
    socket.on('disconnect', () => {
      console.log('User disconnected');
    });
  });
app.post('/data', express.json(), pushData);

// Start the server
server.listen(port, ()=> {
    console.log(`Server listening on http://localhost:${port}/`)
});
