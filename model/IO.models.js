const mongoose = require('mongoose')

const IOSchema = mongoose.Schema({
    date: {
        type: Date,
        default: Date.now()
    },
    IO: String
})

module.exports = mongoose.model('IO', IOSchema)