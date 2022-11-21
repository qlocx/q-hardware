const e = require('child_process')


const run = async () => {
    e.execSync(`nrfjprog --reset`)

    console.log("3sek: Stoppa i batteriet")
    await sleep(3000)

    e.execSync(`nrfjprog --reset`)
    console.log("Du kan testa med appen")
}


const sleep = async (ms) => {
    return new Promise(r => {
        setTimeout(r, ms)
    })
}

run()