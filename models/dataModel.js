const mongoose = require('mongoose');

const dataSchema = new mongoose.Schema({
  temperature: { type: Number, required: true },
  distance: { type: Number, required: true },
  floodWarning: { type: Boolean, required: true },
  timestamp: { type: Date, default: Date.now }
});

const Data = mongoose.model('Data', dataSchema);
module.exports =  Data ;
