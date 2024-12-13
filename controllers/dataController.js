const Data = require('../models/dataModel');
const Joi = require('joi');

module.exports = (io) => {
  // Validation schema
  const dataSchema = Joi.object({
    temperature: Joi.number().required(),
    distance: Joi.number().required(),
    floodWarning: Joi.boolean().required(),
  });

  const fetchData = async (req, res) => {
    try {
      const latestData = await Data.findOne().sort({ timestamp: -1 });
      res.render('index', { data: latestData || null });
    } catch (err) {
      console.error("Error fetching data:", err);
      res.status(500).send({ message: "An error occurred while fetching data." });
    }
  };

  const pushData = async (req, res) => {
    try {
      const { error } = dataSchema.validate(req.body);
      if (error) return res.status(400).send({ message: error.details[0].message });

      const { temperature, distance, floodWarning } = req.body;
      const newData = new Data({ temperature, distance, floodWarning });

      await newData.save();
      console.log("Data saved successfully");

      io.emit('newData', newData);
      res.status(201).send("Data logged successfully");
    } catch (err) {
      console.error("Error pushing data:", err);
      res.status(500).send({ message: "Error logging data." });
    }
  };

  const getAllData = async (req, res) => {
    try {
      const { page = 1, limit = 10 } = req.query;
      const allData = await Data.find()
        .sort({ timestamp: -1 })
        .skip((page - 1) * limit)
        .limit(parseInt(limit));

      res.json(allData);
    } catch (err) {
      console.error("Error fetching all data:", err);
      res.status(500).send({ message: "Error fetching all data." });
    }
  };

  return { fetchData, pushData, getAllData };
};
