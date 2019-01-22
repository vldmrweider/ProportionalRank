#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <string>
#include <map>
#include <algorithm>

//string split function
std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main() {

    std::string line;

    const double minForce = 0.1;
    constexpr double windowLength = 86400.0 * 2.0 * 100.0;
    const double mult = 1E7;

    //force map from stake
    std::map<std::string, double> accountstake;

    //key map account1,account2;
    typedef std::pair<std::string, std::string> Acc2AccKey;
    std::map<Acc2AccKey, double> acc2accActivity;
    //key map account,post;
    typedef std::pair<std::string, std::string> AccPostKey;
    std::map<Acc2AccKey, double> accPostActivity;

    //post author
    std::map<std::string, std::string> postAccount;
    //account Rate
    std::map<std::string, double> accountRate;
    std::map<std::string,double> accountPostRate;
    //comment case
    std::map<std::string,double> accountCommentRate;


    //load account stake
    std::ifstream inputTestSnapshot;
    inputTestSnapshot.open("snapshot.csv");

    std::getline(inputTestSnapshot, line);
    while (std::getline(inputTestSnapshot, line)) {
        auto splitresult = split(line, ';');
        accountstake[splitresult[0]] = std::stod(splitresult[1]) + std::stod(splitresult[2]);
    }
    inputTestSnapshot.close();

    //summa all stake
    const double accountstakeSumma = std::accumulate(std::begin(accountstake), std::end(accountstake), 0.0,
                                                     [](const double previous,
                                                        const std::pair<const std::string, double> &p) {
                                                         return previous + p.second;

                                                     });
    //norm stake step function
    std::for_each(accountstake.begin(), accountstake.end(),
                  [&accountstakeSumma, &minForce](std::pair<const std::string, double> &item) {
                      item.second = (item.second / accountstakeSumma < minForce) ? minForce : item.second /
                                                                                              accountstakeSumma;
                  });

    // load social activity
    const int columnAccount = 2;
    const int columnPost = 3;
    const int columnAction = 4;
    const int columnDistance = 5;
    const std::string actionUpvote = "UPVOTE";
    const std::string actionCreatePost = "OWNERSHIP";

    std::ifstream inputTestActivity;
    inputTestActivity.open("social_activity.csv");


    Acc2AccKey acc2accKey;
    AccPostKey accPostKey;

    while (std::getline(inputTestActivity, line)) {
        auto splitresult = split(line, ';');
        //upvote
        if (splitresult[columnAction] == actionUpvote) {

            if (postAccount[splitresult[columnPost]].empty()) continue;

            acc2accKey.first = splitresult[columnAccount];
            acc2accKey.second = postAccount[splitresult[columnPost]];

            acc2accActivity[acc2accKey] =
                    accountstake[acc2accKey.first] * (windowLength - std::stod(splitresult[columnDistance])) /
                    windowLength;

            accPostKey.first = postAccount[splitresult[columnPost]];
            accPostKey.second = splitresult[columnPost];

            accPostActivity[accPostKey] =
                    accountstake[acc2accKey.first] * (windowLength - std::stod(splitresult[columnDistance])) /
                    windowLength;

            accountRate[postAccount[splitresult[columnPost]]] = 0;

        }
        //Ownership
        if (splitresult[columnAction] == actionCreatePost) {

            postAccount[splitresult[columnPost]] = splitresult[columnAccount];
        }
    }
    inputTestActivity.close();

    //rate calculator
    for (auto &item: acc2accActivity) {

        accountRate[item.first.second] += item.second;

    }
    //norm rate
    const double accountRateSumma = accumulate(accountRate.begin(), accountRate.end(), 0.0,
                                               [](const double acc, std::pair<std::string, double> p) {
                                                   return (acc + p.second);
                                               });

    std::for_each(accountRate.begin(), accountRate.end(),
                  [&accountRateSumma, &mult](std::pair<const std::string, double> &item) {
                      item.second = mult*item.second / accountRateSumma;
                  });




    //calc post rate and cout result
    for (auto &item:accountRate) {
        std::cout << item.first << " = " << item.second << std::endl;

        std::map<std::string, double> accpost;
        for (auto &itempost:accPostActivity)
        {
            if (item.first==itempost.first.first)
                accpost[itempost.first.second]+=itempost.second;
        }

        //norm post rate
        const double accpostSumma = accumulate(accpost.begin(), accpost.end(), 0.0,
                                                   [](const double acc, std::pair<std::string, double> p) {
                                                       return (acc + p.second);
                                                   });

        std::for_each(accpost.begin(), accpost.end(),
                      [&accpostSumma](std::pair<const std::string, double> &item) {
                          item.second = item.second / accpostSumma;
                      });


        for (auto &itemaccpost:accpost)
        {
            accountPostRate[itemaccpost.first] = itemaccpost.second*item.second;
            std::cout<<"   "<<itemaccpost.first<<"="<<accountPostRate[itemaccpost.first]<<std::endl;
        }




    }


}



