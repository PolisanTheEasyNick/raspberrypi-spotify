async function getSpotifyInfoWS() {
    let socket;

    async function connectWebSocket() {
        socket = new WebSocket(PCWSServer);

        socket.addEventListener('open', () => {
            console.log('WebSocket connected');
        });

        socket.addEventListener('message', async function (event) {
            try {
                const spotifyData = JSON.parse(event.data);

                let title = spotifyData.title;
                let artist = spotifyData.artist;
                const artURL = spotifyData.artURL;

                if (title.length > 55) {
                    title = title.substring(0, 54) + "…";
                    document.getElementById('title').style.fontSize = "1em";
                } else {
                    document.getElementById('title').style.fontSize = "1.5em";
                }

                if (artist.length > 15) {
                    artist = artist.substring(0, 15) + "…";
                    document.getElementById('artist').style.fontSize = "1em";
                } else {
                    document.getElementById('artist').style.fontSize = "1.25em";
                }

                console.log("Got title: ", title, ", artist: ", artist, ", artURL: ", artURL);
                document.getElementById('title').innerText = title;

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
                };
                img.src = artURL;
            } catch (error) {
                console.error('Error parsing WebSocket data', error);
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