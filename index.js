const express = require('express');
const app = express();

const http = require('http');
const server = http.createServer(app);
const bodyParser = require('body-parser')
const mongoose = require('mongoose')
const IO = require('./model/IO.models.js')
require('dotenv/config')
const { Server } = require("socket.io");
const io = new Server(server)
const port = process.env.PORT || 8081
const moment = require('moment')

var isOn = true
const today = moment().startOf('day')
var count = 0
const getcount = async () => {
    count = await IO.countDocuments({
        date: { $gte: today.toDate(), $lte: moment(today).endOf('day').toDate() }
    })
    console.log(count)
}
getcount()

//Setup database
mongoose.Promise = global.Promise
mongoose.set('useFindAndModify', false);
const db = mongoose.connection
mongoose.connect(process.env.DB_CONNECTION, { useNewUrlParser: true, useUnifiedTopology: true })
db.on('error', console.error.bind(console, 'MongoDB connection error:'))
//End DB Section

app.set('view engine', 'ejs')
app.set('views', __dirname)
app.use('/public', express.static(__dirname + '/public'));
app.use(bodyParser.urlencoded({ extended: true }));

app.get('/', async (req, res) => {
    res.render('views/index', { count: count })
})
app.post('/api/v1/data', async (req, res) => {
    let date = req.body.date
    console.log(date)
    let data = await IO.find({ date: { $gte: moment(date).startOf('day').toDate(), $lte: moment(date).endOf('day').toDate() } })
    res.json(data)
})
app.post('/', async (req, res) => {
    if (isOn) {
        let recivedIO = { IO: req.body.type }
        saveIO = await new IO(recivedIO).save()
        count += 1
        io.sockets.emit("display", { count: count });
        res.send('On')
    } else {
        res.send('Off')
    }

})
app.post('/toggle', (req, res) => {
    isOn = !isOn
    res.send('ok')
})

io.on('connection', (socket) => {
    console.log('a user connected');
    socket.on('disconnect', () => {
        console.log('user disconnected');
    });
});
server.listen(port, () => {
    console.log('listening on localhost:8081');
});