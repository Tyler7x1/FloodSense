const Data = require('../models/dataModel');

const fetchData = async (req, res) => {
  try {
    // Fetch the latest data from MongoDB to display on the initial page load
    const latestData = await Data.find().sort({ timestamp: -1 }).limit(1);
    if (latestData.length === 0) {
      res.render('index', { data: null});
    } else {
      res.render('index', { data: latestData[0] });
    }
  } catch (err) {
    console.error("Error fetching data:", err);
    res.status(500).send("Error fetching data");
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

const getAllData = async (req, res) => {
  try {
    // Fetch all data from MongoDB
    const allData = await Data.find().sort({ timestamp: -1 });
    if (allData.length === 0) {
      res.status(404).send("No data found");
    } else {
      res.json(allData);
    }
  } catch (err) {
    console.error("Error fetching all data:", err);
    res.status(500).send("Error fetching all data");
  }
};

module.exports = { fetchData, pushData, getAllData };