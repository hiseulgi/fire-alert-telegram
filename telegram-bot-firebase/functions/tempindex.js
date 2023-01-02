const { Telegraf } = require("telegraf");
const { functions } = require("firebase-functions");

// const chatID = functions.config().telegrambot.chatid.val();
const chatID = 632206286;

const bot = new Telegraf(functions.config().telegrambot.key);

bot.hears("hi", (ctx) => ctx.reply("Hey there"));
bot.launch();

exports.bot = functions.https.onRequest((req, res) => {
    bot.handleUpdate(req.body, res);
});

/* ------------------------------------ - ----------------------------------- */
exports.handledb = functions.database.ref("test").onWrite((change, context) => {
    console.log(change.after.val());
    if (change.after.val() == "BAHAYA") {
        bot.telegram.sendMessage(chatID, "BAHAYA");
    }
});
