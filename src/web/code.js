function toBase64(img) {
    const canvas = document.createElement("canvas");
    canvas.width = img.width;
    canvas.height = img.height;
    const ctx = canvas.getContext("2d");
    ctx.drawImage(img, 0, 0);
    return canvas.toDataURL("image/png");
}

const albumArt = document.getElementById('album-art');

function loadImageFromBase64(base64Data) {
    albumArt.src = base64Data;
    albumArt.style.display = "inline";
    document.querySelector('.background').style.backgroundImage = `url(${base64Data})`;
}

async function parseData(spotifyData) {
    try {
        let title = spotifyData.title;
        let artist = spotifyData.artist;
        let album = spotifyData.album;
        const artURL = spotifyData.artURL;

        console.log("Got title: ", title, ", artist: ", artist, ", artURL: ", artURL, ", album: ", album);
        document.getElementById('title').innerText = title;
        document.getElementById('album').innerText = album;
        document.getElementById('artist').innerText = artist;
        const cacheKey = artURL.split('/image/')[1];
        const img = new Image();
        img.crossOrigin = 'anonymous';
        img.onload = function() {
            const base64Data = toBase64(img);
            localStorage.setItem(cacheKey, base64Data);
            loadImageFromBase64(base64Data);
        };
        
        img.onerror = function() {
            albumArt.style.display = "none";
            document.querySelector('.background').style.backgroundImage = "none";
        };
        
        if (artURL) {
            const cachedData = localStorage.getItem(cacheKey);
            if (cachedData) {
                console.log("Loading album art image from local storage!");
                loadImageFromBase64(cachedData);
            } else {
                console.log("Album art not found in local storage, downloading!");
                img.src = artURL;
            }
        } else {
            console.log("No art URL!");
            albumArt.style.display = "none";
            albumArt.src = "";
            document.querySelector('.background').style.backgroundImage = "none";
        }
    } catch (error) {
        console.error('Error parsing WebSocket data', error);
    }
}


var useSpotifyD = false;

async function getSpotifyInfoWS() {
    let socket;

    async function connectWebSocket() {
        socket = new WebSocket(PCWSServer);

        socket.addEventListener('open', () => {
            console.log('WebSocket connected');
        });

        socket.addEventListener('message', async function (event) {
            if(!useSpotifyD) {
                const spotifyData = JSON.parse(event.data);
                console.log("Got message from PC: ", spotifyData)
                parseData(spotifyData);
            }
        });

        socket.addEventListener('close', (event) => {
            console.error('WebSocket closed. Reconnecting in 10 seconds...');
            setTimeout(connectWebSocket, 10000); 
        });
    }

    await connectWebSocket();

}
getSpotifyInfoWS()

async function getSpotifyDInfoWS() {
    let socket;

    async function connectWebSocket() {
        socket = new WebSocket(spotifydWSServer);

        socket.addEventListener('open', () => {
            console.log('WebSocket connected');
            useSpotifyD = true;
        });

        socket.addEventListener('message', async function (event) {
            const spotifyData = JSON.parse(event.data);
            console.log("Got message from spotifyd: ", spotifyData)
            useSpotifyD = spotifyData.isPlaying == "True";
            if(useSpotifyD)
                parseData(spotifyData);
            
        });

        socket.addEventListener('close', (event) => {
            useSpotifyD = false;
            console.error('WebSocket closed. Reconnecting in 10 seconds...');
            setTimeout(connectWebSocket, 10000); 
        });
    }

    await connectWebSocket();

}
getSpotifyDInfoWS()