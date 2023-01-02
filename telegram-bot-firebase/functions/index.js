const { Telegraf } = require("telegraf");
const functions = require("firebase-functions");
const admin = require("firebase-admin");
admin.initializeApp(functions.config().firebase);

var database = admin.database();

const chatIDs = [632206286, 1657898747];
const bot = new Telegraf(functions.config().telegrambot.key);

bot.hears("/start", (ctx) => {
    ctx.reply(
        "Welcome to the bot!\n\nMy name is Fire-sama. ❤️‍🔥\nI will notify you when the fire is detected.\nYou can see all the commands by typing /help.\n\nGlad to see you here! Latom! 🙏🔥"
    );
});
bot.hears("/help", (ctx) =>
    ctx.reply(`Here is what I can do for you:
    /ping - check bot status
    /status - check latest sensor value
    `)
);
bot.hears("/ping", (ctx) => ctx.reply("Ping pong!\nThis bot is running well."));
bot.hears("/status", (ctx) => {
    var ref = database.ref("realData");
    ref.once("value", function (snapshot) {
        var data = snapshot.val();
        var text = `
        ℹ Info Status Alat ℹ\n
        Status: ${data.isWarning}
        Pesan: ${data.warningMsg}
        Timestamp: ${data.timestamp}, ${data.date}\n
        Sensor Api: ${data.fireValue}
        Sensor Gas: ${data.gasValue}
        Sensor Suhu: ${data.temperatureValue}
        `;
        ctx.reply(text);
    });
});
bot.launch();

exports.bot = functions.https.onRequest((req, res) => {
    bot.handleUpdate(req.body, res);
});

exports.newOutputHandler = functions.database
    .ref("realData")
    .onWrite((change, context) => {
        var oldVal = change.before.child("isWarning").val();
        var newVal = change.after.child("isWarning").val();
        console.log(`oldVal: ${oldVal}, newVal: ${newVal}`);

        var warningMsg = change.after.child("warningMsg").val();
        var timestamp = change.after.child("timestamp").val();
        var date = change.after.child("date").val();

        if (newVal != 0 && oldVal != newVal) {
            const text = `⚠️ Warning! ⚠️\n\nStatus ${oldVal} telah berubah menjadi ${newVal}.\n✉️: ${warningMsg}\n🕛: ${timestamp}, ${date}\n\nHati-hati dan terima kasih! ☺️`;
            chatIDs.forEach((chatID) => {
                bot.telegram.sendMessage(chatID, text);
            });
            database
                .ref("metadata/lastChangedName/")
                .set("changed his name from " + oldVal + " to " + newVal);
        }
    });
