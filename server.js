require('dotenv').config();
const express = require('express');
const cors = require('cors');
const { connectToMongo } = require('./connect');
const { fetchData, pushData, latestData, serverListen } = require('./controllers/dataController');

const app = express();

const uri = process.env.MONGO_URI
const port = process.env.PORT;

app.use(cors());  // Enable CORS for all routes

app.set('view engine', 'ejs'); // Set EJS as the view engine
app.set('views', './views');   // Define the views folder

// Connect to MongoDB
connectToMongo(uri);

app.get('/', fetchData);
latestData;
app.post('/data', express.json(), pushData);

// Start the server
serverListen(port);
