function performSearch() {
    const query = document.getElementById('query').value.trim();
    if (!query) {
        alert("Please enter a search term.");
        return;
    }

    const endpoint = "http://<你的EC2公网IP>:5001/search?q=" + encodeURIComponent(query);

    fetch(endpoint)
        .then(response => response.json())
        .then(data => displayResults(data))
        .catch(err => {
            console.error("Search failed:", err);
            document.getElementById("results").innerHTML = "<p>Error retrieving results.</p>";
        });
}

function displayResults(data) {
    const container = document.getElementById("results");
    container.innerHTML = "";

    if (!Array.isArray(data) || data.length === 0) {
        container.innerHTML = "<p>No results found.</p>";
        return;
    }

    data.forEach(hit => {
        const item = document.createElement("div");
        item.classList.add("result");

        item.innerHTML = `
            <h3><a href="${hit.url}" target="_blank">${hit.title}</a></h3>
            <p>${hit.snippet}</p>
            <small>Score: ${hit.score.toFixed(2)}</small>
        `;

        container.appendChild(item);
    });
}
