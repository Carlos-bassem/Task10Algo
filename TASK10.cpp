#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <chrono>
using namespace std;

//convert state to string 
string StateToString(const vector<char>& coins)
{
    return string(coins.begin(), coins.end());
}

//check if all coins removed
bool IsSolved(const vector<char>& coins)
{
    for (char c : coins)
        if (c != '_') return false;
    return true;
}

//check if any heads remain
bool HasHeads(const vector<char>& coins)
{
    for (char c : coins)
        if (c == 'H') return true;
    return false;
}

//flip coin
void Flip(char& coin)
{
    coin = (coin == 'H') ? 'T' : 'H';
}

//backtracking algorithm
bool Solve(vector<char>& coins, set<string>& visited, vector<int>& moves)
{
    // Success: all coins removed
    if (IsSolved(coins))
        return true;

    string state = StateToString(coins);

    // Failure: state already visited
    if (visited.count(state))
        return false;

    // Failure: no heads left but coins remain
    if (!HasHeads(coins))
    {
        visited.insert(state);
        return false;
    }

    // Try removing each head
    for (int i = 0; i < coins.size(); i++)
    {
        if (coins[i] == 'H')
        {
            //save state
            vector<char> backup = coins;

            //remove coin
            coins[i] = '_';

            //flip neighbors
            if (i > 0 && coins[i - 1] != '_')
                Flip(coins[i - 1]);
            if (i < coins.size() - 1 && coins[i + 1] != '_')
                Flip(coins[i + 1]);

            //Recurse
            moves.push_back(i);
            if (Solve(coins, visited, moves))
                return true;

            //Backtrack
            moves.pop_back();
            coins = backup;
        }
    }

    visited.insert(state);
    return false;
}

bool SolveCoinRemoval(vector<char> coins, vector<int>& solution)
{
    int heads = 0;
    for (char c : coins)
        if (c == 'H') heads++;

    if (heads % 2 == 0)
    {
        cout << "No solution: even number of heads\n";
        return false;
    }

    set<string> visited;
    vector<int> moves;

    if (Solve(coins, visited, moves))
    {
        cout << "Solution found: ";
        for (int m : moves)
            cout << m << " ";
        cout << "\n";
        solution = moves;
        return true;
    }

    cout << "No solution found\n";
    return false;
}

//int main()
//{
//vector<pair<int, pair<vector<char>, string>>> tests = {
//    {1, {{{'T', 'H', 'H', 'T', 'H', 'H', 'H'}}, "7 coins, 4 heads (even)"}},
//    {2, {{{'H'}}, "1 head"}},
//    {3, {{{'H', 'H', 'H'}}, "3 heads"}},
//    {4, {{{'H', 'T', 'H', 'T', 'H'}}, "5 coins, 3 heads"}},
//    {5, {{{'H', 'H', 'H', 'H', 'H'}}, "5 heads"}},
//    {6, {{{'H', 'T', 'H'}}, "2 heads (even)"}},
//    {7, {{{'H', 'H', 'H', 'H'}}, "4 heads (even)"}},
//    {8, {{{'T', 'H', 'T', 'H', 'T', 'H', 'T', 'H', 'T', 'H'}}, "10 coins, 5 heads"}},
//    {9, {{{'H', 'T', 'T', 'T', 'T', 'H'}}, "6 coins, 2 heads (even)"}},
//    {10, {{{'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H'}}, "9 heads"}},
//    {11, {{{'H', 'H', 'T', 'H', 'H'}}, "5 coins, 4 heads (even)"}},
//    {12, {{{'H', 'T', 'H', 'H', 'T', 'H'}}, "6 coins, 4 heads (even)"}},
//    {13, {{{'T', 'T', 'H', 'T', 'T', 'H', 'T', 'T', 'H'}}, "9 coins, 3 heads"}},
//    {14, {{{'H', 'T', 'H'}}, "2 heads (even)"}},
//    {15, {{{'T', 'H', 'T'}}, "1 head"}}
//};
//
//for (auto& test : tests)
//{
//    int num = test.first;
//    vector<char> coins = test.second.first;
//    string desc = test.second.second;
//
//    cout << "\n" << string(60, '=') << "\nTEST " << num << ": " << desc << "\n" << string(60, '=') << "\n";
//
//    cout << "Start: ";
//    for (char c : coins)
//        cout << c << " ";
//    cout << "\n";
//
//    vector<int> solution;
//    bool ok = SolveCoinRemoval(coins, solution);
//
//    cout << "Status: " << (ok ? "SUCCESS" : "FAILED") << "\n";
//
//    if (ok)
//        cout << "Moves: " << solution.size() << "\n";
//}
//
//cout << "\n" << string(60, '=') << "\nDONE\n";
//
//return 0;
//}