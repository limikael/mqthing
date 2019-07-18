function jsonRequest(url, o) {
	if (!o)
		o={};

	if (!o.method)
		o.method="GET";

	o.method=o.method.toUpperCase();

	let qa=[];
	for (key in o.form)
		qa.push(key+"="+encodeURIComponent(o.form[key]));

	qs=qa.join("&");
	if (o.method=="GET")
		url+="?"+qs;

	return new Promise((resolve, reject)=>{
		let xhttp=new XMLHttpRequest();
		xhttp.onreadystatechange=function() {
			if (xhttp.readyState==4) {
				if (xhttp.status!=200) {
					reject(new Error("Error"));
					return;
				}

				let o;
				try {
					o=JSON.parse(xhttp.responseText);
				}

				catch (e) {
					reject(e);
				}

				resolve(o);
			}
		}

		xhttp.open(o.method,url,true);

		if (o.method=="POST") {
			xhttp.setRequestHeader('Content-type','application/x-www-form-urlencoded');
			xhttp.send(qs);
		}

		else
			xhttp.send();
	});
}

function el(id) {
	return document.getElementById(id);
}

function onSubmitClick(e) {
	e.preventDefault();

	let options={
		method: "POST",
		form: {
			ssid: el("ssid").value,
			pass: el("ssid-password").value
		}
	};

	jsonRequest("/config",options).then((config)=>{
		console.log("config saved...")
	});
}

function onTimeout() {
	console.log("updating status...");

	jsonRequest("/config").then((o)=>{
		el("status").innerHTML=o.status;

		setTimeout(onTimeout,5000);
	});
}

function main() {
	el("submit-button").onclick=onSubmitClick;
	console.log("running main...");
	jsonRequest("/config").then((o)=>{
		document.title=o.name;
		el("title").innerHTML=o.name;
		el("status").innerHTML=o.status;
		el("ssid").value=o.ssid;
		el("ssid-password").value=o.pass;

		setTimeout(onTimeout,5000);
	});
}

window.addEventListener("load",main);
