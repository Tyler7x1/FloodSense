// Import the mongoose library for MongoDB interactions
const mongoose = require('mongoose');

/**
 * Establishes a connection to a MongoDB instance.
 * 
 * @param {string} url - The URL of the MongoDB instance to connect to.
 * @returns {Promise} A promise that resolves when the connection is established.
 */
const connectToMongo = async function (url) {
    try {
        // Attempt to connect to the MongoDB instance
        const response = await mongoose.connect(url);
        
        // If the connection is successful, log a success message
        if (response) {
            console.log('Connected to MongoDB');
        } else {
            // If the connection fails, throw an error
            throw new Error('Failed to connect to MongoDB');
        }
    } catch (error) {
        // Catch and rethrow any errors that occur during connection
        throw error;
    }
}

// Export the connectToMongo function for use in other modules
module.exports = { connectToMongo };