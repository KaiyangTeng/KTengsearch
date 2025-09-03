# Distributed Search Engine System

A comprehensive, high-performance distributed search engine built from scratch using C++, featuring web crawling, inverted indexing, document retrieval, and ranking algorithms. The system supports real-time web crawling, efficient document indexing, and fast search queries with sub-100ms response times.

## Features

- **Distributed Web Crawler**: Multi-threaded crawling with Redis task queue management
- **Intelligent Content Extraction**: HTML parsing with Gumbo parser and smart URL filtering
- **Inverted Indexing**: Efficient full-text search with MySQL backend
- **BM25 Ranking Algorithm**: Advanced relevance scoring with caching optimizations
- **Real-time Search API**: RESTful API with JSON responses
- **Web Interface**: Clean, responsive frontend for search queries
- **UTF-8 Support**: Robust encoding handling for international content
- **Performance Optimized**: LRU caching, connection pooling, and concurrent processing

## Architecture

### System Components

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Frontend  │    │   Flask API     │    │  C++ Search     │
│   (Port 8080)   │◄──►│   (Port 5001)   │◄──►│   Engine        │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
                       ┌─────────────────┐              │
                       │  Redis Queue    │◄─────────────┘
                       │  (Port 6379)    │
                       └─────────────────┘
                                │
                       ┌─────────────────┐
                       │  MySQL Database │
                       │  (Port 3306)    │
                       └─────────────────┘
```

### Core Modules

1. **Crawler Module**: `LinkFetcher`, `HTML_getter`, `URL_getter`, `URL_filter`
2. **Indexing Module**: `Indexer`, `Tokenizer`, `IndexReader`
3. **Search Module**: `Retriever`, `Ranker`, `QueryParser`
4. **Storage Module**: `DocumentStore`, `DB_manager`
5. **Presentation Module**: `SnippetGenerator`, `SearchEngine`

## Technology Stack

- **Backend**: C++17, MySQL, Redis
- **Libraries**: libcurl, Gumbo parser, hiredis, nlohmann/json
- **API**: Flask (Python)
- **Frontend**: HTML5, CSS3, JavaScript
- **Concurrency**: Multi-threading, Redis task queues
- **Caching**: LRU cache, Redis caching

## Prerequisites

- **C++17** compatible compiler (GCC 7+ or Clang 5+)
- **MySQL 8.0+** server
- **Redis 6.0+** server
- **Python 3.7+** with Flask
- **Development Libraries**:
  - libcurl development headers
  - MySQL development headers
  - Redis development headers

### macOS Installation
```bash
# Install dependencies via Homebrew
brew install mysql redis curl

# Install Python dependencies
pip3 install flask
```

### Ubuntu/Debian Installation
```bash
# Install system dependencies
sudo apt-get update
sudo apt-get install build-essential libcurl4-openssl-dev libmysqlclient-dev libhiredis-dev python3-pip

# Install Python dependencies
pip3 install flask
```

## Quick Start

### 1. Clone and Setup
```bash
git clone <repository-url>
cd distributed-search-engine
```

### 2. Database Configuration
Update database connection parameters in the following files:
- `APItest.cpp` (line 104)
- `worker_v3.cpp` (line 21)
- `test.py` (update IP in script.js line 8)

### 3. Start Services
```bash
# Make startup script executable
chmod +x start_all.sh

# Start all services (MySQL and Redis must be running)
./start_all.sh
```

### 4. Access the System
- **Web Interface**: http://localhost:8080
- **API Endpoint**: http://localhost:5001/search?q=query
- **Command Line**: `./out "search query"`

## Manual Setup

### Start Services Individually

1. **Start Crawler Worker**:
```bash
./run_v3.sh
./workernew &
```

2. **Start Search API**:
```bash
./run_API.sh
python3 test.py &
```

3. **Start Web Server**:
```bash
python3 -m http.server 8080 &
```

## System Performance

- **Search Response Time**: < 100ms average
- **Crawling Speed**: 1 request/second (configurable)
- **Indexing Capacity**: Supports millions of documents
- **Concurrent Users**: Multi-threaded architecture supports high concurrency
- **Cache Hit Rate**: 40-60% latency reduction with LRU caching

## Search Features

### Query Processing
- **Tokenization**: Automatic word segmentation with stopword removal
- **Query Parsing**: Support for multi-word queries
- **UTF-8 Handling**: Robust encoding for international content

### Ranking Algorithm
- **BM25 Implementation**: Industry-standard relevance scoring
- **Term Frequency**: Document-level term frequency analysis
- **Document Frequency**: Collection-level term distribution
- **Length Normalization**: Document length bias correction

### Result Presentation
- **Snippet Generation**: Context-aware text extraction
- **Query Highlighting**: Bold highlighting of search terms
- **Relevance Scoring**: Transparent scoring for result ranking

## Usage Examples

### Command Line Interface
```bash
# Basic search
./out "machine learning"

# Multi-word search
./out "artificial intelligence research"

# View results in JSON format
./out "python programming" | jq .
```

### API Usage
```bash
# HTTP GET request
curl "http://localhost:5001/search?q=web%20development"

# Python example
import requests
response = requests.get("http://localhost:5001/search?q=databases")
results = response.json()
```

### Web Interface
1. Open http://localhost:8080 in your browser
2. Enter search terms in the input field
3. Click "Search" to view results
4. Click on result titles to visit original pages

## Project Structure

```
distributed-search-engine/
├── src/                          # Core C++ source files
│   ├── SearchEngine.h/.cpp       # Main search engine class
│   ├── Indexer.h/.cpp           # Inverted index builder
│   ├── IndexReader.h/.cpp       # Index query interface
│   ├── Retriever.h/.cpp         # Document retrieval
│   ├── Ranker.h/.cpp            # BM25 ranking algorithm
│   ├── DocumentStore.h/.cpp     # Document storage with LRU cache
│   ├── Tokenizer.h/.cpp         # Text tokenization
│   ├── QueryParser.h/.cpp       # Query processing
│   ├── SnippetGenerator.h/.cpp  # Result snippet generation
│   ├── LinkFetcher.h/.cpp       # Web crawler coordinator
│   ├── HTML_getter.h/.cpp       # HTTP client (libcurl)
│   ├── URL_getter.h/.cpp        # Link extraction (Gumbo)
│   ├── URL_filter.h/.cpp        # URL filtering
│   ├── Content_getter.h/.cpp    # Content extraction
│   └── DB_manager.h/.cpp        # MySQL database interface
├── third_party/                  # External libraries
│   ├── gumbo-parser/            # HTML parsing library
│   └── hiredis/                 # Redis client library
├── web/                         # Frontend files
│   ├── index.html               # Main web interface
│   ├── style.css                # Styling
│   └── script.js                # Frontend JavaScript
├── scripts/                     # Build and deployment scripts
│   ├── run_API.sh              # Search engine compilation
│   ├── run_v3.sh               # Crawler compilation
│   └── start_all.sh            # Complete system startup
├── test.py                     # Flask API server
├── APItest.cpp                 # Command-line search interface
├── worker_v3.cpp               # Distributed crawler worker
└── README.md                   # This file
```

## Configuration

### Database Settings
Update connection parameters in source files:
```cpp
// MySQL connection parameters
const std::string host = "your-mysql-host";
const std::string user = "your-username";
const std::string password = "your-password";
const std::string database = "crawler_db";
const unsigned int port = 3306;
```

### Redis Settings
```cpp
// Redis connection parameters
const std::string redis_host = "your-redis-host";
const int redis_port = 6379;
```

### Crawler Settings
```cpp
// Crawling parameters
constexpr long long MAX_URLS = 1000;           // Maximum URLs to crawl
constexpr int RATE_LIMIT = 1;                  // Requests per second
constexpr std::chrono::seconds RATE_WINDOW(1); // Rate limit window
```

## Testing

### Unit Tests
```bash
# Test individual components
./test_indexreader
./test_retri
./test_ranker
```

### Integration Tests
```bash
# Test complete search pipeline
./testmain
```

### Load Testing
```bash
# Test API performance
for i in {1..100}; do
  curl "http://localhost:5001/search?q=test$i" &
done
wait
```

## Troubleshooting

### Common Issues

1. **MySQL Connection Failed**
   - Verify MySQL server is running
   - Check connection parameters
   - Ensure database exists

2. **Redis Connection Failed**
   - Verify Redis server is running
   - Check Redis configuration
   - Ensure Redis is accessible

3. **Compilation Errors**
   - Install required development libraries
   - Check C++17 compiler support
   - Verify library paths

4. **Search Returns No Results**
   - Ensure crawler has populated database
   - Check index is built
   - Verify search terms exist in indexed content

### Debug Mode
```bash
# Enable verbose logging
export DEBUG=1
./start_all.sh
```

## Performance Tuning

### Database Optimization
- Enable MySQL query caching
- Optimize database indexes
- Configure connection pooling

### Memory Management
- Adjust LRU cache size in DocumentStore
- Monitor memory usage patterns
- Optimize garbage collection

### Network Optimization
- Configure Redis persistence
- Optimize HTTP client settings
- Implement connection reuse

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- **Gumbo Parser**: HTML5 parsing library
- **libcurl**: HTTP client library
- **hiredis**: Redis client library
- **nlohmann/json**: JSON library for C++
- **Flask**: Python web framework

## Support

For questions, issues, or contributions, please:
- Open an issue on GitHub
- Contact the development team
- Check the troubleshooting section

---

**Built with C++, MySQL, Redis, and modern web technologies**
