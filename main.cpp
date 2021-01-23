#include <iostream>
#include <boost/filesystem.hpp>
#include <sstream>
#include <map>


class Broker{
private:
    class Account {
    private:
        std::string lastDate;
        size_t fileNum;
    public:
        inline static void setDate(Account& account, const std::string& date) {
            account.fileNum++;
            if (atoi(date.c_str()) > atoi(account.lastDate.c_str())) {
                account.lastDate = date;
            }
        }
        explicit Account(const std::string& date): lastDate(date), fileNum(0){};
        Account() = default;
        [[nodiscard]] size_t getFileNum() const { return fileNum; }
        [[nodiscard]] std::string getLastDate() const { return lastDate; }
    };
    std::map<std::string, Account> accounts; // Pair : accountID / Account
    std::string name;
public:
    static Broker InspectSingleBroker(const boost::filesystem::path& p);
    explicit Broker(const std::string& n): name(n){}
    friend std::ostream& operator<<(std::ostream& os, const std::vector<Broker>& brokers);
    static std::vector<Broker> Inspect(const boost::filesystem::path& p, std::ostream& os);
    inline static bool isRightFile(const boost::filesystem::path& fileName);
};

inline bool is_separator(char c) { return c == '_'; }
inline bool not_separator(char c) { return !is_separator(c); }

void splitString(const std::string &text, std::string& ID, std::string& date) {
    std::vector<std::string> ret;
    for (auto i = text.cbegin(); i != text.cend();) { // balance_00100003_20180928
        i = std::find_if(i, text.end(), not_separator);
        auto j = std::find_if(i, text.end(), is_separator);
        ret.emplace_back(i, j); // substr(i, j)
        i = j;
    }
    if (ret.size() == 3) {
        ID = ret[1];
        date = ret[2];
    }
}

Broker Broker::InspectSingleBroker(const boost::filesystem::path &p) {
    Broker broker(p.filename().string());
    for (const auto& x : boost::filesystem::directory_iterator{p})
    {
        if(!is_directory(x)) {
            if (isRightFile(x.path())) {
                std::string accountID;
                std::string accountDate;

                splitString(x.path().stem().string(), accountID, accountDate);

                if(broker.accounts.find(accountID) != broker.accounts.end()) {
                    Account::setDate(broker.accounts[accountID], accountDate);
                } else {
                    broker.accounts.insert(std::make_pair<std::string, Account>(accountID.c_str(), Account(accountDate)));
                }
            }
            }
        }
    return broker;
}

std::vector<Broker> Broker::Inspect(const boost::filesystem::path& p, std::ostream& os) {
    std::vector<Broker> back;
    for (const auto& x : boost::filesystem::directory_iterator{p})
    {
        if(!is_directory(x)) {
            if (isRightFile(x.path())) {
                os << x.path().filename().string() << "\n";
            }
        } else if (is_directory(x)) {
            std::stringstream out;
            out << x.path().filename().string() << "\n";
            Inspect(x.path(), out);
            if(out.str() != x.path().filename().string() + "\n"){
                os << out.str();
                back.emplace_back(Broker::InspectSingleBroker(x.path()));
            }
        } else {
            os << "This is not a regular file or dir!\n";
        }
    }
    return back;
}

std::ostream &operator<<(std::ostream &os, const std::vector<Broker>& brokers) {
    if (brokers.empty()) {
        os << "Brokers array is empty!\n";
    } else {
        os << "============================================BROKERS SUMMARY============================================\n";
        for (const auto& broker : brokers) {
            for (const auto& account : broker.accounts) {
                os << "broker:" << broker.name + "\t"
                   << "account:" << account.first << "\tfiles:" << account.second.getFileNum()
                   << "\tlastdate:" << account.second.getLastDate() << "\n";
            }
        }
    }
    return os;
}

bool Broker::isRightFile(const boost::filesystem::path &fileName) {
    return (fileName.stem().extension().string() != ".old" &&
            fileName.filename().string().find("balance") != std::string::npos);
}

int main(int argc, char** argv) {
//    const boost::filesystem::path p{"/home/lamp/Programming/Lab4/lab04-boost_filesystem-yuna/misc"};
    if (argc == 1) {
        std::cout << Broker::Inspect(".", std::cout);
    } else if (argc == 2) {

        std::cout << Broker::Inspect(argv[1], std::cout);
    } else {
        std::cout << "Too many arguments!";
    }
}


/*
 *  ftp -> broker -> account -> last date
 */