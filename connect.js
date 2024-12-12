const mongoose = require('mongoose');

const connectToMongo = async function (url) {
    const response = await mongoose.connect(url);
    if(!response) throw Error
    else
    console.log('Connected to MongoDB');
}

module.exports = { connectToMongo };