#!/usr/bin/node

const express=require('express');
const app=express();
const port=3000;
const fs=require("fs");

let config={
	ssid: "Hello World",
	pass: "testing"
};

function htmlEntities(str) {
	return String(str)
		.replace(/&/g, '&amp;')
		.replace(/</g, '&lt;')
		.replace(/>/g, '&gt;')
		.replace(/"/g, '&quot;');
}

app.use(express.urlencoded({extended: true}));

app.get('/', (req, res) => {
	console.log("Serving page");

	let template=fs.readFileSync("page.html").toString();

	template=template.replace("{title}",htmlEntities("Hello"));
	template=template.replace("{title}",htmlEntities("Hello"));
	template=template.replace("{ssid}",htmlEntities("da\"<net"));
	template=template.replace("{pass}",htmlEntities("oioioio"));

	template=template.replace("{formdisplay}","block");
	template=template.replace("{messagedisplay}","none");

	res.end(template);
});

/*app.get("/script", (req, res)=>{
	console.log("Serving script");
	res.end(fs.readFileSync("script.js").toString());
});*/

/*app.all("/config", (req, res)=>{
	console.log("Serving config: ",req.body);

	if (req.body.hasOwnProperty("ssid"))
		config.ssid=req.body.ssid;

	if (req.body.hasOwnProperty("pass"))
		config.pass=req.body.pass;

	o={
		name: "My Thing",
		status: "connecting "+Math.round(Math.random()*100),
		ssid: config.ssid,
		pass: config.pass
	};
	res.end(JSON.stringify(o));
});*/

app.listen(port, () => console.log(`Page test listening on port ${port}!`))