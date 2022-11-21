const fs = require('fs')

const run = () => {
    console.log(JSON.parse(fs.readFileSync('build/config.json').toString()))
}

run()