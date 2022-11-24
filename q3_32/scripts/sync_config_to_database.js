var fs = require('fs'), e = require('child_process')
const fetch = require('node-fetch')

console.log(process.argv)

// fs.writeFileSync(__dirname + '/bleid.txt', JSON.parse(fs.readFileSync(__dirname + '/build/config.json')).public_key.substring(0, 8))
const config = JSON.parse(fs.readFileSync(`${__dirname}/../build/config.json`).toString())
const bleid = config.public_key.substring(0, 8)

console.log(config)

fs.writeFileSync(`${__dirname}/bleid.txt`, bleid)

try {
    fs.writeFileSync(`/home/robert/Qlocx/qtestserver/controller.json`, JSON.stringify(config))
    console.log(e.execSync(`lp -d DYMO_LabelManager_PnP ${__dirname}/bleid.txt -o landscape -o page-top=15 -o media=Custom.12x24mm`).toString())
} catch (e) {
    // console.log(e)
}


const [, , destinedSystem = 'qlocx'] = process.argv

let URL
const JWT = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJxbG9jeHByb2dyYW1tZXIxIiwiaWF0IjoiIn0.uZ39hx-S4rcrApB4_FoYLNe4ecXHW2i--oA2djYc3eE'
if (destinedSystem === "qlocx") {
    URL = 'https://0rwvhut4cf.execute-api.eu-north-1.amazonaws.com/production/controller'
} else if (destinedSystem === "iboxen") {
    URL = 'https://0xmcj67sa0.execute-api.eu-north-1.amazonaws.com/staging/controller'
}

// URL = 'http://localhost:4001/controller'
const sleep = async (ms = 1000) => {
    return new Promise(resolve => {
        setTimeout(resolve, ms)
    })
}

const sendToAPI = async () => {
    const body = {
        publicKey: config.public_key,
        privateKey: config.private_key,
        board: "q3_32"
    }

    const result = await fetch(URL, {
        method: "POST",
        headers: {
            'Content-Type': 'application/json',
            'Authorization': JWT
        },
        body: JSON.stringify(body)
    })
    const jsonResult = await result.json()

    console.log(jsonResult)

    if (result.status !== 200) {
        console.log(`Qlocx sync: status ${result.status}, ${result.statusText}, ${jsonResult.message}`)
    } else {
        await sleep(1000)
        try {
            console.log(e.execSync("nrfjprog --reset").toString())
        } catch { }
        console.log("\n===== Stoppa nu i batteriet i kretskortet =====")
	console.log(`\n===== Kretskortet har fått ID: ${config.public_key.substring(0,8)} ===\n`)
    }
}

sendToAPI()
