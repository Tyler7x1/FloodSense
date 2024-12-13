require('dotenv').config();
const express = require('express');
const http = require('http');
const cors = require('cors');
const socketIo = require('socket.io');
const { connectToMongo } = require('./connect');
const Data = require('./models/dataModel');

// Initialize the app
const app = express();
const server = http.createServer(app);
const io = socketIo(server);

// Load environment variables
const uri = process.env.MONGO_URI;
const port = process.env.PORT;

// Middleware
app.use(express.json());
app.use(cors()); // Enable CORS for all routes

// Set up view engine
app.set('view engine', 'ejs'); // Set EJS as the view engine
app.set('views', './views');   // Define the views folder

// Connect to MongoDB
connectToMongo(uri);

// Import controllers and pass `io` for real-time updates
const { fetchData, pushData, getAllData } = require('./controllers/dataController')(io);

// Define routes
app.get('/', fetchData);          // Home route
app.post('/data', pushData);      // Route to add data
app.get('/data', getAllData);     // Route to get all data

// WebSocket connection for real-time updates
io.on('connection', async (socket) => {
  console.log('A user connected');

  try {
    // Fetch the most recent data from the database and send it to the client
    const latestData = await Data.findOne().sort({ timestamp: -1 });
    if (latestData) {
      socket.emit('newData', latestData); // Emit the most recent data
    }
  } catch (err) {
    console.error("Error fetching latest data:", err);
  }

  socket.on('disconnect', () => {
    console.log('User disconnected');
  });
});

// Start the server
server.listen(port, () => {
  console.log(`Server listening on http://localhost:${port}/`);
});
