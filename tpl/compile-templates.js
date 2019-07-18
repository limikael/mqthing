#!/usr/bin/node

const fs=require("fs");
const minify=require("minify");

function cify(s) {
	s=s.replace(/"/g,'\\"');
	s=s.replace(/\n/g,'\\n');
	return '"'+s+'"';
}

function chunkify(s, l) {
	let res=[];

	if (!l)
		l=80;

	while (s.length) {
		res.push(s.substr(0,l));
		s=s.substr(l);
	}

	return res;
}

function mkconst(name, text) {
	return (
		"const char "+name+"[] PROGMEM=\n    "+
		chunkify(text,65).map(cify).join("\n    ")+
		";\n"
	);
}

async function main() {
	let s="";

//	let page=await minify(__dirname+"/page.html");
	let page=fs.readFileSync(__dirname+"/page.html").toString();
	while (page.indexOf("  ")>=0)
		page=page.replace("  "," ");
	s+=mkconst("MQTHING_PAGE",page);

/*	let script=await minify(__dirname+"/script.js");
	s+="\n"+mkconst("MQTHING_SCRIPT",script);*/

	fs.writeFileSync(__dirname+"/../MQTHING_TEMPLATES.h",s);
}

main();