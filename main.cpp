#include <iostream>
#include <queue>
#include <string>
#include <ctime>
#include <unordered_map>
#include <utility>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <fstream>
#include <string>


using namespace std;

// Custom structure to hold data
int order_id = 0;

class Order {
public:
    int orderId;
    std::string client_order_id;
    std::string instrument;
    int side;
    int quantity;
    double price;

    void newOrder() const;
    void pfillOrder(int value, double price) const;
    void fillOrder(double price) const;
    void setQuantity(int value);

    Order(std::string  clientOrderId, std::string  instrumentName, int orderSide,
          int orderQuantity, double orderPrice)
            : client_order_id(std::move(clientOrderId)),
              instrument(std::move(instrumentName)),
              side(orderSide),
              quantity(orderQuantity),
              price(orderPrice)
    {
        orderId = ++order_id;
    }
};

string getTime(){
    stringstream ss;
    // Get current system time with milliseconds
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // Convert to local time
    std::tm local_time = *std::localtime(&now_c);

    // Format time as string and append to output stream
    ss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}


void Order::newOrder() const {
    std::ofstream file("execution_rep.csv", std::ios_base::app);
    if (file.is_open()) {
        file << "ord" << orderId << "," << client_order_id << "," << instrument << "," << side << "," << "new" << "," << quantity << "," << price << ",," << getTime() << std::endl;
        file.close();
    } else {
        // Handle file open error
        std::cout << "Failed to open file." << std::endl;
    }
}

void Order::fillOrder(double ex_price) const {
    std::ofstream file("execution_rep.csv", std::ios_base::app);
    if (file.is_open()) {
        file << "ord" << orderId << "," << client_order_id << "," << instrument << "," << side << "," << "fill" << "," << quantity << "," << ex_price << ",," << getTime() << std::endl;
        file.close();
    } else {
        // Handle file open error
        std::cout << "Failed to open file." << std::endl;
    }
}

void Order::pfillOrder(int value, double ex_price) const {
    std::ofstream file("execution_rep.csv", std::ios_base::app);
    if (file.is_open()) {
        file << "ord" << orderId << "," << client_order_id << "," << instrument << "," << side << "," << "pfill" << "," << value << "," << ex_price << ",," << getTime() << std::endl;
        file.close();
    } else {
        // Handle file open error
        std::cout << "Failed to open file." << std::endl;
    }
}


void Order::setQuantity(int value){quantity = value;}


// Custom comparison function for Buy side (ascending order by price, time priority for equal prices)
struct CompareBuy {
    bool operator()(const Order& order1, const Order& order2) const {
        if (order1.price != order2.price)
            return order1.price < order2.price;

        // If prices are equal, prioritize by timestamp
        return order1.orderId > order2.orderId;
    }
};

// Custom comparison function for Sell side (descending order by price, time priority for equal prices)
struct CompareSell {
    bool operator()(const Order& order1, const Order& order2) const {
        if (order1.price != order2.price)
            return order1.price > order2.price;

        // If prices are equal, prioritize by timestamp
        return order1.orderId > order2.orderId;
    }
};


void sellhandler(std::priority_queue<Order, std::vector<Order>, CompareSell> sellOrders[],
                 std::priority_queue<Order, std::vector<Order>, CompareBuy> buyOrders[],
                 int flowerPositions,
                 const Order& order) {

    if(sellOrders[flowerPositions].top().price > buyOrders[flowerPositions].top().price){
        sellOrders[flowerPositions].top().newOrder();
    }
    else{

        while (sellOrders[flowerPositions].top().price <= buyOrders[flowerPositions].top().price){

            if (sellOrders[flowerPositions].top().quantity == buyOrders[flowerPositions].top().quantity){
                sellOrders[flowerPositions].top().fillOrder(buyOrders[flowerPositions].top().price);
                buyOrders[flowerPositions].top().fillOrder(buyOrders[flowerPositions].top().price);
                sellOrders[flowerPositions].pop();
                buyOrders[flowerPositions].pop();
                break;
            }

            else if(buyOrders[flowerPositions].top().quantity > sellOrders[flowerPositions].top().quantity){

                Order topBuyOrder = buyOrders[flowerPositions].top();  // get a clone
                int newQuantity = topBuyOrder.quantity - sellOrders[flowerPositions].top().quantity;

                //change the quantity
                topBuyOrder.setQuantity(newQuantity);

                //pfill the buy order
                topBuyOrder.pfillOrder(sellOrders[flowerPositions].top().quantity, buyOrders[flowerPositions].top().price);

                //pop the top and add the clone to top

                sellOrders[flowerPositions].top().fillOrder(buyOrders[flowerPositions].top().price);

                buyOrders[flowerPositions].pop();
                buyOrders[flowerPositions].push(topBuyOrder);  // Step 3

                sellOrders[flowerPositions].pop();
                break;
            }

            else {
                Order topSellOrder = sellOrders[flowerPositions].top();  // get a clone
                int newQuantity = topSellOrder.quantity - buyOrders[flowerPositions].top().quantity;

                //change the quantity
                topSellOrder.setQuantity(newQuantity);

                //pfill the buy order
                topSellOrder.pfillOrder(buyOrders[flowerPositions].top().quantity, buyOrders[flowerPositions].top().price);

                //pop the top and add the clone to top
                sellOrders[flowerPositions].pop();
                sellOrders[flowerPositions].push(topSellOrder);  // Step 3

                buyOrders[flowerPositions].top().fillOrder(buyOrders[flowerPositions].top().price);
                buyOrders[flowerPositions].pop();

                if (buyOrders[flowerPositions].empty()){
                    break;
                }

            }
        }
    }
}


void buyhandler(std::priority_queue<Order, std::vector<Order>, CompareSell> sellOrders[],
                std::priority_queue<Order, std::vector<Order>, CompareBuy> buyOrders[],
                int flowerPositions,
                const Order& order) {

    if(sellOrders[flowerPositions].top().price > buyOrders[flowerPositions].top().price){
        buyOrders[flowerPositions].top().newOrder();
    }
    else{

        while (sellOrders[flowerPositions].top().price <= buyOrders[flowerPositions].top().price){

            if (sellOrders[flowerPositions].top().quantity == buyOrders[flowerPositions].top().quantity){
                sellOrders[flowerPositions].top().fillOrder(sellOrders[flowerPositions].top().price);
                buyOrders[flowerPositions].top().fillOrder(sellOrders[flowerPositions].top().price);
                sellOrders[flowerPositions].pop();
                buyOrders[flowerPositions].pop();
                break;
            }

            else if(buyOrders[flowerPositions].top().quantity > sellOrders[flowerPositions].top().quantity){

                Order topBuyOrder = buyOrders[flowerPositions].top();  // get a clone
                int newQuantity = topBuyOrder.quantity - sellOrders[flowerPositions].top().quantity;

                //change the quantity
                topBuyOrder.setQuantity(newQuantity);

                //pfill the buy order
                topBuyOrder.pfillOrder(sellOrders[flowerPositions].top().quantity, sellOrders[flowerPositions].top().price);

                //pop the top and add the clone to top
                buyOrders[flowerPositions].pop();
                buyOrders[flowerPositions].push(topBuyOrder);  // Step 3

                sellOrders[flowerPositions].top().fillOrder(sellOrders[flowerPositions].top().price);
                sellOrders[flowerPositions].pop();

                if (buyOrders[flowerPositions].empty()){
                    break;
                }
            }

            else {
                Order topSellOrder = sellOrders[flowerPositions].top();  // get a clone
                int newQuantity = topSellOrder.quantity - buyOrders[flowerPositions].top().quantity;

                //change the quantity
                topSellOrder.setQuantity(newQuantity);

                //pfill the buy order
                topSellOrder.pfillOrder(buyOrders[flowerPositions].top().quantity, sellOrders[flowerPositions].top().price);

                buyOrders[flowerPositions].top().fillOrder(sellOrders[flowerPositions].top().price);

                sellOrders[flowerPositions].pop();
                sellOrders[flowerPositions].push(topSellOrder);  // Step 3

                buyOrders[flowerPositions].pop();

                break;

            }
        }
    }
}


// Function to insert a buy order into the appropriate priority queue
void insertBuyOrder(std::priority_queue<Order, std::vector<Order>, CompareSell> sellOrders[],
                    std::priority_queue<Order, std::vector<Order>, CompareBuy> buyOrders[],
                    const std::unordered_map<std::string, int>& flowerPositions,
                    const Order& order) {

    int position;

    if (flowerPositions.find(order.instrument) != flowerPositions.end()) {
        position = flowerPositions.at(order.instrument);
        buyOrders[position].push(order); // Buy side

        if (sellOrders[position].empty()){
            order.newOrder();
        } else{
            buyhandler(sellOrders, buyOrders, position, order); // call sell handler
        }
    }

}

// Function to insert a sell order into the appropriate priority queue
void insertSellOrder(std::priority_queue<Order, std::vector<Order>, CompareSell> sellOrders[],
                     std::priority_queue<Order, std::vector<Order>, CompareBuy> buyOrders[],
                     const std::unordered_map<std::string, int>& flowerPositions,
                     const Order& order) {
    int position;

    if (flowerPositions.find(order.instrument) != flowerPositions.end()) {

        position = flowerPositions.at(order.instrument);

        sellOrders[position].push(order); // Sell side

        if (buyOrders[position].empty()){
            order.newOrder();
        } else{
            sellhandler(sellOrders, buyOrders, position, order); // call sell handler
        }

    }

}

bool isDouble(const std::string& str) {
    int dotCount = 0;
    for (char c : str) {
        if (!isdigit(c)) {
            if(c!='.' || dotCount >0){
                return false;
            }
            else{
                dotCount ++;
            }
        }
    }
    return true;
}

bool isInteger(const std::string& str) {
    for (char c : str) {
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

void raiseError(const std::string &Client_ID, const std::string &Instrument, const std::string &side,
                const std::string &quantity, const std::string &price, const std::exception& e) {

    std::ofstream file("execution_rep.csv", std::ios_base::app);
    if (file.is_open()) {
        file << ++order_id << ","<< Client_ID << "," << Instrument << "," << side << ","<< "Rejected" << "," << quantity << ",";
        file << price << "," << e.what() << "," << getTime() << std::endl;
        file.close();
    } else {
        std::cout << "Failed to open file." << std::endl;
    }
}


int main() {
    const int numInstruments = 5;  // Number of different instruments

    std::priority_queue<Order, std::vector<Order>, CompareBuy> buyOrders[numInstruments];
    std::priority_queue<Order, std::vector<Order>, CompareSell> sellOrders[numInstruments];

    // Flower positions
    std::unordered_map<std::string, int> flowerPositions = {
            { "Rose", 0 },
            { "Lavender", 1 },
            { "Lotus", 2 },
            { "Tulip", 3 },
            { "Orchid", 4 }
    };

    // Sample data
    auto start_time = std::chrono::high_resolution_clock::now();

    std::ifstream file("orders.csv");

    std::string line;

    //ignore the fist line
    std::getline(file, line);

    std::string clientOrderID, instrument , side, quantity, price;

    std::ofstream outfile("execution_rep.csv", std::ios::trunc);
    if (outfile.is_open()) {
        // Add a header to the file
        outfile << "Cl.Ord.Id" << ",";
        outfile << "Instrument" << ",";
        outfile << "Side" << ",";
        outfile << "Exec. Status" << ",";
        outfile << "Quantity" << ",";
        outfile << "Price" << ",";
        outfile << "time" << ",";
        outfile << "reason" << ",";
        outfile << std::endl;

    } else {
        std::cerr << "Unable to open file." << std::endl;
    }

    // Do the process and write the execution_rep.csv

    std::ofstream outfile2("execution_rep.csv", std::ios::app);
    if (outfile2.is_open()) {
        // Read each line of the CSV file
        while (std::getline(file, line)) {
            std::stringstream ss(line);

            // Parse each value in the line
            std::getline(ss, clientOrderID, ',');
            std::getline(ss, instrument, ',');
            std::getline(ss, side, ',');
            std::getline(ss, quantity, ',');
            std::getline(ss, price, ',');


            int m_side, m_quantity;
            double m_price;

            // Convert side, quantity, and price to integers
            try {

                switch (instrument[0]) {
                    case 'R':
                        if (instrument == "Rose") {
                            break;
                        }
                    case 'L':
                        if (instrument == "Lavender") {
                            break;
                        } else if (instrument == "Lotus") {
                            break;
                        }
                    case 'T':
                        if (instrument == "Tulip") {
                            break;
                        }
                    case 'O':
                        if (instrument == "Orchid") {
                            break;
                        }

                    default:
                        throw std::invalid_argument("Invalid instrument");
                }

                if (clientOrderID.empty()){
                    throw std::invalid_argument("Invalid clientOrderID");
                }

                // validate the side argument

                if (!isInteger(side) || side.empty()) {
                    throw std::invalid_argument("Invalid Side");
                }

                m_side = std::stoi(side);
                if(m_side != 1 && m_side != 2 ){
                    throw std::invalid_argument("Invalid Side");
                }
                // validate quantity

                if (!isInteger(quantity) || quantity.empty()) {
                    throw std::invalid_argument("Invalid quantity");
                }

                m_quantity = std::stoi(quantity);
                if (not(m_quantity >= 10 && m_quantity <= 1000 && m_quantity%10 == 0)){
                    throw std::invalid_argument("Invalid quantity");
                }

                // validate price

                if (!isDouble(price) || price.empty()){
                    throw std::invalid_argument("Invalid price");
                }


                m_price = std::stod(price);

                if(m_price <= 0){
                    throw std::invalid_argument("Invalid price");
                }

                Order order = { clientOrderID, instrument, m_side, m_quantity, m_price };


                if(m_side == 1){
                    insertBuyOrder(sellOrders, buyOrders, flowerPositions, order);
                } else{
                    insertSellOrder(sellOrders, buyOrders, flowerPositions, order);
                }
            }
            catch (const std::exception& e) {
//                // Handle the error
                raiseError(clientOrderID, instrument, side, quantity, price, e);
            }


        }

    } else {
        std::cerr << "Unable to open file." << std::endl;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Print the time difference
    std::cout << "Time difference: " << time_diff.count() << " milliseconds" << std::endl;


    return 0;
}