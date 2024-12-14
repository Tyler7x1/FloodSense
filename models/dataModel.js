// Import the mongoose library for MongoDB interaction
const mongoose = require('mongoose');

// Define the schema for the data collection
const dataSchema = new mongoose.Schema({
  // Temperature value (required)
  temperature: { type: Number, required: true },
  // Distance value (required)
  distance: { type: Number, required: true },
  // Flood warning flag (required)
  floodWarning: { type: Boolean, required: true },
  // Timestamp (defaults to current date and time)
  timestamp: { type: Date, default: Date.now }
});

// Create a mongoose model from the schema
const Data = mongoose.model('Data', dataSchema);

// Export the Data model for use in other modules
module.exports = Data;