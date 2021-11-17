/*module.exports = {
    DiscordToken: function() {
       return '[token]';
    }
    ,
    RedditSignIn: function() {
        return {
            usergent: 'Discord bot by trackpadpro',
            userame: 'Def13Bot',
            passord: '[password]',
            clientd: '[clientID]',
            clientecret: '[clientSecret]'
        };
    }
 }*/

 //Create an imin.js file in the same path with correct [values]

 const LogMeIn = require('../imin');
const Discord = require('discord.js');
const client = new Discord.Client();
client.login(LogMeIn.DiscordToken());
const snoowrap = require('snoowrap');
let redder = LogMeIn.RedditSignIn();
let reddit = new snoowrap({
    userAgent: redder.usergent,
    username: redder.userame,
    password: redder.passord,
    clientId: redder.clientd,
    clientSecret: redder.clientecret
});
let readyup = Date.now();
let annoyotron = Date.now();
let commandotron = Date.now();
const helpmeplz = require('./helpme');
let instructions = helpmeplz.instructionManual();
//client.user.setPresence({
//    game: {name: 'Type \"help\" for commands.'},
//    status: 'dnd'
//}) 
//.then(console.log)
//.catch(console.error);

client.on('message', message => {
    if (Date.now() < commandotron + 1080) return;
    if (message.author.bot)
        return;
    const args = message.content.split(' ');
    const command = args.shift().toLowerCase();
    if (command == 'help') {
        message.channel.send(instructions);
    }
    if (command == 'ping') {
        message.reply("pong");
    } //"good practice is just using single quotes" - keaton
    if (command == 'valpal') {
        var sentence = ""
        args.forEach(function(arghhh){sentence += `${arghhh} `});
        message.channel.send(sentence);
    }
    if (command == 'r') {
        var thanos = parseInt(args[2]);
        if (1 <= thanos && thanos <= 5) {
            var postcounter = thanos;
        }
        else {
            //message.channel.send('Please use an integer between 1 and 5 to select the number of posts that you would like to search for.');
            var postcounter = 1;
        }
        switch (args[1]) {
            case 'hot':
            reddit.getHot(args[0], {limit: postcounter})
            //.then(console.log);
            .then(function(Listing) {
                //var content = ""
                //Listing.forEach(function(Submission){content += `\`\`\`${Submission['title']}\`\`\`${Submission['url']}`});
                //message.channel.send(content);
                Listing.forEach(function(Submission) {
                    message.channel.send(`\`\`\`${Submission['title']}\`\`\`${Submission['url']}`);
                });
                //console.log(Listing)
            });
            break;
            case 'random':
            reddit.getRandomSubmission(args[0])
            .then(function(Submission) {
                message.channel.send(`\`\`\`${Submission['title']}\`\`\`${Submission['url']}`)
                //console.log(Submission)
            });
            break;
            case 'top':
            reddit.getTop(args[0], {limit: postcounter})
            .then(function(Listing) {
                Listing.forEach(function(Submission) {
                    message.channel.send(`\`\`\`${Submission['title']}\`\`\`${Submission['url']}`);
                });
            });
            break;
            case 'controversial':
            reddit.getControversial(args[0], {limit: postcounter})
            .then(function(Listing) {
                Listing.forEach(function(Submission) {
                    message.channel.send(`\`\`\`${Submission['title']}\`\`\`${Submission['url']}`);
                });
            });
            break;
            case 'rising':
            reddit.getRising(args[0], {limit: postcounter})
            .then(function(Listing) {
                Listing.forEach(function(Submission) {
                    message.channel.send(`\`\`\`${Submission['title']}\`\`\`${Submission['url']}`);
                });
            });
            break;
            case 'new':
            reddit.getNew(args[0], {limit: postcounter})
            .then(function(Listing) {
                Listing.forEach(function(Submission) {
                    message.channel.send(`\`\`\`${Submission['title']}\`\`\`${Submission['url']}`);
                });
            });
            break;
            default: //hot
            switch (args[0]) {
                case args[0]:
                reddit.getHot(args[0], {limit: postcounter})
                .then(function(Listing) {
                    Listing.forEach(function(Submission) {
                        message.channel.send(`\`\`\`${Submission['title']}\`\`\`${Submission['url']}`);
                    });
                });
                break;
                default:
                reddit.getHot({limit: postcounter})
                .then(function(Listing) {
                    Listing.forEach(function(Submission) {
                        message.channel.send(`\`\`\`${Submission['title']}\`\`\`${Submission['url']}`);
                    });
                });
            }
        }
    }
    commandotron = Date.now();
});
//original_date
//if (current_time < original_date + 5s){
//    do stuff
//    original_date = current_time;
//}
client.on('message',message => {
    if (Date.now() > annoyotron + 3600000) {
        if (message.author.bot) return;
        if (message.author.presence.status == 'online') return;
        if (message.author.presence.status == 'idle' || 'dnd' || 'offline') { //&& - and
            message.reply("It really pushes my buttons and grinds my gears when people don't have their status as online when they really are. Just a little robot joke for ya.");
        }
        annoyotron = Date.now()
    } 
});

client.on('message', message => {
    if (Date.now() > readyup + 720000) {
        if (message.author.bot) return;
        const gayme = message.author.presence.game;
        if (gayme == null) return;
        if (gayme.name == 'Counter-Strike Global Offensive') {
            message.channel.send("Stop typin' and get back to fraggin'!");
        }
        if (gayme.name == 'League of Legends') {
            message.channel.send(`LoL, ${message.author.username}, why play a MOBA when you could be drinking boba?`);
        }
        if (gayme.name == 'Hearthstone') {
            message.channel.send("Get back to me once you stop playing that children's card game");
        }
        if (gayme.name == 'Overwatch') {
            message.channel.send("Let's be honest, you only play Overwatch for the \"plot\"");
        }
        if (gayme.name == 'DOTA 2') {
            message.channel.send("DOTA is even worse than League... Unless it's the song. That's a classic.");
        }  
        if (gayme.name == 'SMITE') {
            message.channel.send("Playing Smite? You must be Billy.");
        }
        if (gayme.name == 'Enter the Gungeon') {
            message.channel.send(`What exotic tastes you have in games, ${message.author.username}.`);
        }
        if (gayme.name == 'Spotify') {
            message.channel.send("Instead of Spotify, why don't you look up my boy Keaton on SoundCloud?");
        }
        readyup = Date.now()
    }
});

client.on('error', error => {
    console.error(error);
});
process.on('unhandledRejection', err => console.error(`Uncaught Promise Rejection: \n${err.stack}`));
process.on('uncaughtException', err => console.error(`There was an error here: ${err.stack}`));
client.on('ready', () => {
    console.log('Ready!');
});
