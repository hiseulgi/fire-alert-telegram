const functions = require("firebase-functions");
const admin = require("firebase-admin");
admin.initializeApp(functions.config().firebase);

var database = admin.database();

exports.helloWorld = functions.https.onRequest((request, response) => {
    response.send("Hello from Firebase!");
});

exports.textToLength = functions.https.onRequest((request, response) => {
    var text = request.query.text;
    var textLength = text.length;
    response.send(String(textLength));
});

exports.newNodeDetected = functions.database
    .ref("test")
    .onWrite((change, context) => {
        var oldName = change.before.val();
        var newName = change.after.val();
        var userId = context.params.userId;
        database
            .ref("metadata/lastChangedName/")
            .set(
                userId + " changed his name from " + oldName + " to " + newName
            );

        return null;
    });

exports.pushDataEveryMinute = functions.pubsub
    .schedule("every 1 minutes")
    .onRun((context) => {
        var date = new Date();
        database.ref("metadata/lastUpdate/").set(date.getTime());

        return null;
    });
