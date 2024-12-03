#include "protocol.hpp"

std::string ServerMessage::encode() {
    std::ostringstream oss;
    oss << playerCount << " " << isRunning << " ";
    for (const auto &data : rankings) {
        oss << data.first.first << " " << data.first.second << " " << data.second << " ";
    }
    return oss.str();
}

ServerMessage ServerMessage::decode(const std::string &data) {
    std::istringstream iss(data);
    int count;
    iss >> count;
    ServerMessage message(count);

    iss >> message.isRunning;
    while (!iss.eof()) {
        std::string name;
        int score, timestamp;
        if (iss >> score >> timestamp >> name) {
            message.rankings.insert({{score, timestamp}, name});
        }
    }
    return message;
}

// client message encoding and decoding
std::string ClientMessage::encode() {
    std::ostringstream oss;
    oss << data.name << " " << data.score << " " << data.timestamp;
    return oss.str();
}

ClientMessage ClientMessage::decode(const std::string &data) {
    std::istringstream iss(data);
    std::string name;
    int score, timestamp;
    iss >> name >> score >> timestamp;
    return ClientMessage(name, score, timestamp);
}

// test script
// int main() {
//     std::string encoded;

//     // Client Message test script
//     std::cout << "Client message test:" << std::endl;
//     ClientMessage client(1, 300);

//     // Client that was created
//     std::cout << "ID: " << client.data.id
//               << ", Score: " << client.data.score
//               << ", Timestamp: " << client.data.timestamp << std::endl;

//     // Encoding message to send to server
//     encoded = client.encode();
//     std::cout << "Encoded: " << encoded << std::endl;

//     // Decoding message sent from client
//     ClientMessage decodedClient = ClientMessage::decode(encoded);
//     std::cout << "Decoded ID: " << decodedClient.data.id
//               << ", Score: " << decodedClient.data.score
//               << ", Timestamp: " << decodedClient.data.timestamp << std::endl;

//     // Server Message test script
//     std::cout << "\nServer message test:" << std::endl;
//     ServerMessage server(3);

//     // Adding players
//     // I order the players by score (descending), and then by timestamp (ascending)
//     server.insert(1, 100, 12);
//     server.insert(2, 200, 13);
//     server.insert(3, 200, 14);

//     // Display players in descending order by score
//     for (const auto &player : server.allData) {
//         std::cout << "ID: " << player.id
//                   << ", Score: " << player.score
//                   << ", Timestamp: " << player.timestamp << std::endl;
//     }

//     // Encoding message to send to client
//     encoded = server.encode();
//     std::cout << "Encoded " << encoded << std::endl;

//     // Decoding message sent from server
//     ServerMessage decodedServer = ServerMessage::decode(encoded);
//     std::cout << "Player Count: " << decodedServer.playerCount << std::endl;
//     for (const auto &player : decodedServer.allData) {
//         std::cout << "Decoded ID: " << player.id
//                   << ", Score: " << player.score << std::endl;
//     }

//     return 0;
// }
