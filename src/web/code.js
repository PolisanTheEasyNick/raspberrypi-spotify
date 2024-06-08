function openDatabase() {
    return new Promise((resolve, reject) => {
        const request = indexedDB.open('SpotifyImageCache', 1);
        request.onerror = (event) => reject('Database error: ' + event.target.errorCode);
        request.onsuccess = (event) => resolve(event.target.result);
        request.onupgradeneeded = (event) => {
            const db = event.target.result;
            db.createObjectStore('images', { keyPath: 'id' });
        };
    });
}

function saveImageToIndexedDB(id, base64Data) {
    return openDatabase().then((db) => {
        return new Promise((resolve, reject) => {
            const transaction = db.transaction(['images'], 'readwrite');
            const store = transaction.objectStore('images');
            store.put({ id: id, data: base64Data });
            transaction.oncomplete = () => resolve();
            transaction.onerror = (event) => reject('Transaction error: ' + event.target.errorCode);
        });
    });
}

function getImageFromIndexedDB(id) {
    return openDatabase().then((db) => {
        return new Promise((resolve, reject) => {
            const transaction = db.transaction(['images']);
            const store = transaction.objectStore('images');
            const request = store.get(id);
            request.onsuccess = (event) => resolve(event.target.result ? event.target.result.data : null);
            request.onerror = (event) => reject('Request error: ' + event.target.errorCode);
        });
    });
}

const albumArt = document.getElementById('album-art');

function loadImageFromBase64(base64Data) {
    albumArt.src = base64Data;
    albumArt.style.display = "inline";
    document.querySelector('.background').style.backgroundImage = `url(${base64Data})`;
}

function toBase64(img) {
    const canvas = document.createElement("canvas");
    canvas.width = img.width;
    canvas.height = img.height;
    const ctx = canvas.getContext("2d");
    ctx.drawImage(img, 0, 0);
    return canvas.toDataURL("image/png");
}


async function fetchYouTubeThumbnail(title, artist) {
    const query = `${title} ${artist}`;
    const url = `https://www.googleapis.com/youtube/v3/search?part=snippet&q=${encodeURIComponent(query)}&key=${youtubeDataAPIKEY}&type=video&videoEmbeddable=true`;

    try {
        const response = await fetch(url);
        const data = await response.json();
        if (data.items && data.items.length > 0) {
            return data.items[0].snippet.thumbnails.high.url;
        }
    } catch (error) {
        console.error('Error fetching data from YouTube', error);
    }
    return null;
}

async function parseData(spotifyData) {
    try {
        let title = spotifyData.title;
        let artist = spotifyData.artist;
        let album = spotifyData.album;
        let artURL = spotifyData.artURL;
        let fromYouTube = false;

        console.log("Got title: ", title, ", artist: ", artist, ", artURL: ", artURL, ", album: ", album);
        document.getElementById('title').innerText = title;
        document.getElementById('album').innerText = album;
        document.getElementById('artist').innerText = artist;
        
        if (!artURL && youtubeDataAPIKEY) {
            fromYouTube = true;
            console.log("No art URL provided, fetching from YouTube.");
            artURL = await fetchYouTubeThumbnail(title, artist);
        }
        
        if (artURL) {
            let cacheKey;
            if (artURL.includes('/vi/')) {
                cacheKey = artURL.split('/vi/')[1].split('/')[0];
            } else {
                cacheKey = artURL.split('/image/')[1];
            }

            const img = new Image();
            //img.crossOrigin = window.location.protocol + '//' + window.location.host;
            img.crossOrigin = "anonymous";
            img.onload = async function() {
                const base64Data = toBase64(img);
                await saveImageToIndexedDB(cacheKey, base64Data);
                loadImageFromBase64(base64Data);
            };
            img.onerror = function() {
                albumArt.style.display = "none";
                document.querySelector('.background').style.backgroundImage = "none";
            };

            const cachedData = await getImageFromIndexedDB(cacheKey);
            if (cachedData) {
                console.log("Loading album art image from IndexedDB!");
                loadImageFromBase64(cachedData);
            } else {
                console.log("Album art not found in IndexedDB, downloading!");
                if (fromYouTube)
                  img.src = corsProxy + artURL;
                else
                  img.src = artURL;
            }
        } else {
            console.log("No art URL found!");
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