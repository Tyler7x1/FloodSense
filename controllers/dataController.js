const Data = require('../models/dataModel'); // Import the Data model
const Joi = require('joi'); // Import Joi for data validation

module.exports = (io) => {
  // Joi validation schema for incoming data
  const dataSchema = Joi.object({
    temperature: Joi.number().required(),  // Temperature must be a number and is required
    distance: Joi.number().required(),    // Distance must be a number and is required
    floodWarning: Joi.boolean().required(), // FloodWarning must be a boolean and is required
  });

  /**
   * Fetches the latest data entry from the database and renders it on the "index" view.
   */
  const fetchData = async (req, res) => {
    try {
      // Retrieve the latest document from the Data collection, sorted by timestamp in descending order
      const latestData = await Data.findOne().sort({ timestamp: -1 });

      // Render the index view and pass the latest data or null if no data is available
      res.render('index', { data: latestData || null });
    } catch (err) {
      console.error("Error fetching data:", err);
      // Respond with an error message if data fetching fails
      res.status(500).send({ message: "An error occurred while fetching data." });
    }
  };

  /**
   * Validates and saves incoming sensor data to the database, then emits it to connected clients.
   */
  const pushData = async (req, res) => {
    try {
      // Validate the incoming request body against the defined schema
      const { error } = dataSchema.validate(req.body);
      if (error) {
        // If validation fails, return a 400 status with the error message
        return res.status(400).send({ message: error.details[0].message });
      }

      // Destructure the validated data from the request body
      const { temperature, distance, floodWarning } = req.body;

      // Create a new data entry using the Data model
      const newData = new Data({ temperature, distance, floodWarning });

      // Save the data to the database
      await newData.save();
      console.log("Data saved successfully");

      // Emit the new data to all connected clients via the WebSocket
      io.emit('newData', newData);

      // Respond with a success message
      res.status(201).send("Data logged successfully");
    } catch (err) {
      console.error("Error pushing data:", err);
      // Respond with an error message if data saving fails
      res.status(500).send({ message: "Error logging data." });
    }
  };

  /**
   * Fetches paginated data from the database and returns it as JSON.
   * Query parameters:
   *  - page: The page number (default is 1)
   *  - limit: The number of documents per page (default is 10)
   */
  const getAllData = async (req, res) => {
    try {
      const { page = 1, limit = 10 } = req.query; // Get pagination parameters from query string

      // Fetch paginated data sorted by timestamp in descending order
      const allData = await Data.find()
        .sort({ timestamp: -1 })          // Sort by newest first
        .skip((page - 1) * limit)         // Skip documents for previous pages
        .limit(parseInt(limit));          // Limit the number of results per page

      // Respond with the retrieved data as JSON
      res.json(allData);
    } catch (err) {
      console.error("Error fetching all data:", err);
      // Respond with an error message if data fetching fails
      res.status(500).send({ message: "Error fetching all data." });
    }
  };

  // Return the defined functions for use in routes
  return { fetchData, pushData, getAllData };
};
