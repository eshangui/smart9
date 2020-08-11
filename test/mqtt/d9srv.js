var mqtt = require('./');

var up_topic_list = ['/smart9/up/heartbeat/', '/smart9/up/upload/'];
var down_topic_list = ['/smart9/down/heartbeat/', '/smart9/down/upload/'];

var options = {
  username: 'admin',
  password: 'password',
}

console.log('server start');
var client = mqtt.connect('mqtt://121.36.3.243:61613', options);

console.log('connected');

//sub heartbeat
client.subscribe(up_topic_list[0]);
console.log('execute sub heartbeat');

console.log('listening');


client.on('message', function (topic, message) {
  console.log('get message');
  setTimeout(function (topic, message){
    if (topic == up_topic_list[0]) {
      //handle heatbeat
      console.log('get heartbeat:' + message);
      client.subscribe(up_topic_list[1]+message);
      console.log('subscribe:' + up_topic_list[0]+message);
      client.publish(down_topic_list[0]+message, message);
      return;
    }

    var address = topic.substring(topic.length - 16, topic.length);
    console.log('address = ' + address);
    var topic_only = topic.substring(0, topic.length - 16);
    console.log('message:' + message);

    switch (topic_only) {
      case up_topic_list[1]:
        console.log('topic = ' + topic_only);
        var publish_topic = down_topic_list[1] + address;
        client.publish(publish_topic, message);
        break;
      
      default:
        console.log('unknown message')
    }
  },1000, topic, message);
  
})

