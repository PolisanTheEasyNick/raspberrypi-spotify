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
        const albumArt = document.getElementById('album-art');
        albumArt.style.opacity = 0;
        const img = new Image();
        img.onload = function() {
            albumArt.src = artURL;
            albumArt.style.opacity = 1;
            document.querySelector('.background').style.backgroundImage = `url(${artURL})`;
        };
        img.onerror = function() {
            console.log("New image failed to load. Keeping the old one.");
            if (!albumArt.src) {
                albumArt.style.opacity = 0;
                document.querySelector('.background').style.backgroundImage = "none";
                document.querySelector('.background').style.backgroundColor = "black";
            }
        };
        if(artURL) {
          img.src = artURL;
        } else {
            console.log("No art URL!");
            albumArt.style.opacity = 0;
            document.querySelector('.background').style.backgroundImage = "none";
            document.querySelector('.background').style.backgroundColor = "black";
        }
    } catch (error) {
        console.error('Error parsing WebSocket data', error);
    }
}

async function getSpotifyInfoWS() {
    let socket;

    async function connectWebSocket() {
        socket = new WebSocket(PCWSServer);

        socket.addEventListener('open', () => {
            console.log('WebSocket connected');
        });

        socket.addEventListener('message', async function (event) {
            const spotifyData = JSON.parse(event.data);
            console.log("Got message from PC: ", spotifyData)
            parseData(spotifyData);
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
        });

        socket.addEventListener('message', async function (event) {
            const spotifyData = JSON.parse(event.data);
            console.log("Got message from spotifyd: ", spotifyData)
            parseData(spotifyData);
        });

        socket.addEventListener('close', (event) => {
            console.error('WebSocket closed. Reconnecting in 10 seconds...');
            setTimeout(connectWebSocket, 10000); 
        });
    }

    await connectWebSocket();

}
getSpotifyDInfoWS()