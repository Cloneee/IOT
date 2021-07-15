var data = []
var socket = io();
socket.on("display", count => {
    document.getElementById("count").innerHTML = Object.values(count)
    noti();
})

function noti(){
    let d = $('<div/>')
    $(d).addClass('alert alert-success animate__animated animate__fadeInDown')
    .html('New customer at ' + moment().format('hh:mm:ss'))
}

$(document).ready(() => {
    let date = moment().format('YYYY-MM-DD');
    $('#datepicker').val(date)
    $.ajax('/api/v1/data', {
        type: 'POST',  // http method
        data: { date: date },  // data to submit
        success: function (result) {
            for (const [key, value] of Object.entries(result)) {
                data.push(value.date)
                $('#main-table').append(`
                    <tr>
                        <td>${key}</td>
                        <td>${moment(value.date).format("dddd, MMMM Do YYYY, h:mm:ss a")}</td>
                    </tr>
                `)
            }
        },
        error: function (err) {
            console.warn(err)
        }
    });
})
$(function () {
    $("#datepicker").datepicker({
        dateFormat: "yy-mm-dd"
    });
});
$('#datepicker').change(() => {
    let date = $('#datepicker').val()
    data = []
    $('#main-table').empty()
    $.ajax('/api/v1/data', {
        type: 'POST',  // http method
        data: { date: date },  // data to submit
        success: function (result) {
            for (const [key, value] of Object.entries(result)) {
                data.push(value.date)
                $('#main-table').append(`
                    <tr>
                        <td>${key}</td>
                        <td>${moment(value.date).format("dddd, MMMM Do YYYY, h:mm:ss a")}</td>
                    </tr>
                `)
            }
        },
        error: function (err) {
            console.warn(err)
        }
    });
})
$('#scroll-top').click(() => {
    window.scrollTo({ top: 0, behavior: 'smooth' });
})
$('#scroll-bottom').click(() => {
    window.scrollTo({ top: $(document).height(), behavior: 'smooth' });
})