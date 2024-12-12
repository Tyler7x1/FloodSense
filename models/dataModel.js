const mongoose = require('mongoose');

const dataSchema = new mongoose.Schema({
  temperature: { type: Number, required: true },
  distance: { type: Number, required: true },
  floodWarning: { type: Boolean, required: true },
  timestamp: { type: Date, default: Date.now }
});

module.exports = mongoose.model('Data', dataSchema);
