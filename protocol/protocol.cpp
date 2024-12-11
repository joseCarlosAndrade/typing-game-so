#include "protocol.hpp"

std::string ServerMessage::encode() {
    std::ostringstream oss;
    oss << int(type) << " ";
    switch (type) {
    case ServerMessageType::START :
        oss << playerCount;
        break;
    
    case ServerMessageType::RANKING :
        oss << playerCount << " " << isRunning << " ";
        for (const auto &data : rankings) {
            oss << data.second.first << " " << data.second.second << " " << data.first << " ";
        } 
        break;
    
    case ServerMessageType::PHRASE :
        std::cout << "encapsulating phrase: " << phrase << std::endl;
        oss << playerCount << " " << phrase;
        break;

    case ServerMessageType::END_GAME :
        oss << playerCount << " ";
        for (const auto &data : rankings) {
            oss << data.second.first << " " << data.second.second << " " << data.first << " ";
        }
        break;

    default:
        perror("ERROR ServerMessageType not reconigzed");
        break;
    }

    return oss.str();
}

ServerMessage ServerMessage::decode(const std::string &data) {
    std::istringstream iss(data);
    ServerMessageType in_type;
    int temp1, temp2;
    std:: string temps;
    iss >> temp1;
    in_type = ServerMessageType(temp1);
    ServerMessage message;
    message.type = in_type;

    switch (in_type) {
    case ServerMessageType::START :
        iss >> message.playerCount;
        break;
    
    case ServerMessageType::RANKING :
        iss >> message.playerCount >> message.isRunning;
        for (int i = 0; i < message.playerCount; i++){
            // Reads the ranking, following the conventions in encode
            iss >> temp1 >> temp2 >> temps;
            message.rankings.insert({temps, {temp1, temp2}});
        }
        break;
    
    case ServerMessageType::PHRASE :
        iss >> message.playerCount >> message.phrase;
        break;

    case ServerMessageType::END_GAME :
        iss >> message.playerCount;
        for (int i = 0; i < message.playerCount; i++){
            // Reads the ranking, following the conventions in encode
            iss >> temp1 >> temp2 >> temps;
            message.rankings.insert({temps, {temp1, temp2}});
        }
        break;

    default:
        perror("ERROR ServerMessageType not reconigzed");
        break;
    }
    
    
    return message;
}

// client message encoding and decoding
std::string ClientMessage::encode() {
    std::ostringstream oss;
    oss << int(type) << " ";
    switch (type)
    {
    case ClientMessageType::READY :
        oss << data.name;
        break;
    
    case ClientMessageType::PROGRESS :
        oss << data.name << " " << data.score << " " << data.timestamp;    
        break;

    case ClientMessageType::DONE :
        oss << data.timestamp;
        break;
    
    
    default:
        perror("ERROR ClientMessageType not reconigzed");
        break;
    }
    
    return oss.str();
}

ClientMessage ClientMessage::decode(const std::string &data) {
    std::istringstream iss(data);
    ClientMessageType in_type;
    int temp1;
    std:: string temps;
    iss >> temp1;
    in_type = ClientMessageType(temp1);
    ClientMessage message(in_type);
     switch (in_type)
    {
    case ClientMessageType::READY :
        iss >> message.data.name;
        break;
    
    case ClientMessageType::PROGRESS :
        iss >> message.data.name >> message.data.score >> message.data.timestamp;
        break;

    case ClientMessageType::DONE :
        iss >> message.data.timestamp;
        break;
    
    
    default:
        perror("ERROR ClientMessageType not reconigzed");
        break;
    }
    
    
    return message;
    
}
