// Import required modules and configure environment variables
require('dotenv').config();
const express = require('express'); // Express.js framework
const http = require('http'); // HTTP module
const cors = require('cors'); // CORS middleware
const socketIo = require('socket.io'); // Socket.IO library
const { connectToMongo } = require('./connect'); // MongoDB connection module
const Data = require('./models/dataModel'); // Data model

// Initialize the Express app and HTTP server
const app = express();
const server = http.createServer(app);
const io = socketIo(server); // Initialize Socket.IO

// Load environment variables from .env file
const uri = process.env.MONGO_URI; // MongoDB connection URI
const port = process.env.PORT; // Server port

// Middleware configuration
app.use(express.json()); // Enable JSON parsing
app.use(cors()); // Enable CORS for all routes

// Set up view engine
app.set('view engine', 'ejs'); // Set EJS as the view engine
app.set('views', './views'); // Define the views folder

// Connect to MongoDB
connectToMongo(uri);

// Import controllers and pass `io` for real-time updates
const { fetchData, pushData, getAllData } = require('./controllers/dataController')(io);

// Define routes
app.get('/', fetchData); // Home route
app.post('/data', pushData); // Route to add data
app.get('/data', getAllData); // Route to get all data

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

  // Handle socket disconnection
  socket.on('disconnect', () => {
    console.log('User disconnected');
  });
});

// Start the server
server.listen(port, () => {
  console.log(`Server listening on http://localhost:${port}/`);
});