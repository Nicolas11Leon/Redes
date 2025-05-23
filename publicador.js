const mqtt = require('mqtt');
const axios = require('axios');

const broker = 'mqtt://192.168.1.x'; // Tu IP local
const client = mqtt.connect(broker);

let ultimoBTC = 0;
let ultimoSOL = 0;

const umbralBTC = 100; // cambia mínimo para publicar
const umbralSOL = 2;

client.on('connect', () => {
  console.log('🟢 Conectado al broker MQTT');

  setInterval(async () => {
    try {
      const res = await axios.get(
        'https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,solana&vs_currencies=usd'
      );

      const btc = res.data.bitcoin.usd;
      const sol = res.data.solana.usd;

      if (Math.abs(btc - ultimoBTC) > umbralBTC) {
        client.publish('/cripto/BTC/precio', btc.toString());
        console.log(`📈 BTC: $${btc}`);
        ultimoBTC = btc;
      }

      if (Math.abs(sol - ultimoSOL) > umbralSOL) {
        client.publish('/cripto/SOL/precio', sol.toString());
        console.log(`📉 SOL: $${sol}`);
        ultimoSOL = sol;
      }

    } catch (error) {
      console.error('❌ Error al obtener datos:', error.message);
    }
  }, 15000); // cada 15 segundos
});
