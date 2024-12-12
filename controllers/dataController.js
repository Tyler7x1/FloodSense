const express = require('express');
const http = require('http');
const Data = require('../models/dataModel');
const app = express();
const server = http.createServer(app);
const socketIo = require('socket.io');

const io = socketIo(server);

const fetchData = async (req, res) => {
    try {
      // Fetch the latest data from MongoDB to display on the initial page load
      const latestData = await Data.find().sort({ timestamp: -1 }).limit(1);
      res.render('index', { data: latestData[0] || null }); // Render the EJS template with data or null if no data found
    } catch (err) {
      console.error("Error fetching data:", err);
      res.render('index', { data: null }); // Render with null data in case of error
    }
  };
  
  const pushData = async (req, res) => {
    try {
      const { temperature, distance, floodWarning } = req.body;
  
      // Create a new data document based on the incoming data
      const newData = new Data({
        temperature,
        distance,
        floodWarning,
        timestamp: new Date(),
      });
  
      await newData.save(); // Save the new data to the database
      console.log("Data saved successfully");
  
      io.emit('newData', newData); // Emit the new data to all connected clients
      res.status(201).send("Data logged successfully");
    } catch (err) {
      console.error("Error pushing data:", err);
      res.status(500).send("Error logging data");
    }
  };
  
  const latestData = async (req, res) => {
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
  };

  const serverListen = (port) => {
    try {
      server.listen(port);
      console.log(`Server is running on https://localhost:${port}/`);
    } catch (err) {
      console.error("Error starting server:", err);
    }
  }

  module.exports = { fetchData, pushData, latestData, serverListen };
  