async function getSpotifyInfoWS() {
    let socket;

    async function connectWebSocket() {
        socket = new WebSocket("ws://127.0.0.1:4831");

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
                    document.getElementById('title').style.fontSize = "2em";
                } else {
                    document.getElementById('title').style.fontSize = "3em";
                }

                if (artist.length > 15) {
                    artist = artist.substring(0, 15) + "…";
                    document.getElementById('artist').style.fontSize = "2em";
                } else {
                    document.getElementById('artist').style.fontSize = "2.5em";
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

    function adjustFontSize() {
        const container = document.querySelector('.container');
        const title = document.getElementById('title');
        const artist = document.getElementById('artist');

        let titleFontSize = 4;
        let artistFontSize = 2.5;

        title.style.fontSize = `${titleFontSize}em`;
        artist.style.fontSize = `${artistFontSize}em`;

        while ((title.scrollWidth > container.clientWidth || title.scrollHeight > container.clientHeight) && titleFontSize > 1) {
            titleFontSize -= 0.1;
            title.style.fontSize = `${titleFontSize}em`;
        }

        while ((artist.scrollWidth > container.clientWidth || artist.scrollHeight > container.clientHeight) && artistFontSize > 1) {
            artistFontSize -= 0.1;
            artist.style.fontSize = `${artistFontSize}em`;
        }
    }

}
getSpotifyInfoWS()