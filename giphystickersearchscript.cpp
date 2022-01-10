/* This C++ program is an interactive console app that performs various actions to find
stickers from giphy through custom searches. This application utilizes boost.asio in order
to make requests to the giphy api. All searches and search results are kept in a STL list
that contains vectors that represents each page of results with the result urls. Each page
is 20 different sticker results.

Author: Logan Kraver
Date: 1/10/2022*/

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <algorithm>

//global list of vectors representing search requests
std::list< std::vector<std::string> > searchlist;

//enumerator to specific the different actions for the application to add clarity
enum applicationActions {
    APPLICATION_QUIT,
    APPLICATION_SEARCH_NEW,
    APPLICATION_SEARCH_NEXT,
    APPLICATION_PRINT_LAST,
    APPLICATION_PRINT_SEARCHES,
    APPLICATION_PRINT_SPECIFIC,
    APPLICATION_RANK
};

//global bool variable to determine whether the app is running
bool runApp = true;

std::string searchGiphy(std::string const& search, unsigned int page) {
    /*
    Search function utilizing boost.asio to get a json of the search data using the giphy API
    
    Parameter search is a string representing the custom search by the user
    Parameter page is an unsigned int representing the page of the search

    Returns the json for the search term and page
    */
    boost::asio::ip::tcp::iostream stream;

    //request to giphy API
    stream.connect("api.giphy.com", "http");
    stream    << "GET /v1/stickers/search?api_key=vQ3ODLXxGd1CZ5iFTqhTcG1sM40AdKFi&limit=20&q=" << search << "&offset=" << page*20 << " HTTP/1.1\r\n";
    stream    << "Host: api.giphy.com\r\n";
    stream    << "Connection: close\r\n\r\n" << std::flush;

    //add the json string to a vector and add that vector to a list
    std::vector<std::string> result;
    result.emplace_back(search);
    searchlist.insert(searchlist.end(), 1, result);

    //return the json string
    std::ostringstream os;
    os << stream.rdbuf();
    return os.str();
}

std::string lowercase(std::string s)
{
    /*
    Helper function to make a string lowercase 
    
    Parameter s is a string to turn lowercase

    Outputs a new string that is all lowercase
    */
    std::string newstring = "";
    for (char const &c: s)
    {
        newstring += std::tolower(c);
    }
    return newstring;
} 

int findItem(std::string data, std::string a)
{
    /*
    Searches a json string for a specific element and adds it to the search results list

    Parameter data is a string that represents the json
    Parameter a is the element to search for
    */

    std::string backslash = "\"";
    std::string attribute =  backslash + a + backslash;
    bool remainingresults = true;
    unsigned int lastpos = 0;    

    //loop to search until no results remain
    while (remainingresults)
    {
        //positions of the element and result
        size_t pos1 = data.find(attribute, lastpos);
        size_t pos2 = data.find('\"', pos1 + attribute.length());
        size_t pos3 = data.find('\"', pos2 + 1);
        
        std::string element = data.substr(pos2 + 1, (pos3) - (pos2 + 1));
        
        //if the final position wraps around to the beginning end the search
        if (lastpos > pos3)
            return 0;
        
        //set the last pos for next iteration
        lastpos = pos3;
        
        //add element if it is a correct link
        if (element.find("https://giphy.com/stickers/") < element.length())
            searchlist.back().emplace_back(element);
    }
    return -1;
    
}

void printlist(std::list<std::vector <std::string> > list)
{
    /*
    Helper function to print all of the searches in a list with specification for pages 
    and searches

    List is a STL list that contains vectors that contain strings that represent the 
    different pages of searches
    */

    //counter variable to count how many pages have been looped through
    unsigned int counter = 0;

    for (auto const &i: list)
    {
        unsigned int page = 0;
        std::list<std::vector <std::string> >::iterator it = list.begin();
        
        //loop based on the amount of pages that have already been printed
        for (int y = 0; y < counter + 1; y++)
        {
            //tick up page count if the starting strings matchup
            if (i.front() == (*it).front())
                page = page + 1;
            
            //tick up iterator of the list
            std::advance(it, 1);
        }
        std::cout << "-------------------------------------------------------\n";
        std::cout << "Search: " << i.front() << " Page: " << page << std::endl;

        //print the url strings
        for (auto const &e: i)
        {
            if (e != i.front())
                std::cout << e << std::endl;
        }

        //increase counter after printing one page
        counter = counter + 1;
    }
}

applicationActions userAction()
{
    /*
    Helper function to display and allow the user to input their action

    Returns the enum for applicationActions given the user input
    */
    unsigned int action;
    while (true)
    {
        //clear terminal and print all application options
        system("clear");
        std::cout << "---------------------------------------\n";
        std::cout << "[0] Quit\n";
        std::cout << "[1] Search New Term\n";
        std::cout << "[2] Next Page\n";
        std::cout << "[3] Print Last Search Results\n";
        std::cout << "[4] Print All Searches\n";
        std::cout << "[5] Print Specific Page Results\n";
        std::cout << "[6] Rank of Stickers\n";
        std::cout << "What action would you like to choose: ";
        std::cin >> action;

        //return correct applicationAction given input
        if (action == 0) {
            return APPLICATION_QUIT;
        }
        else if (action == 1) {
            return APPLICATION_SEARCH_NEW;
        }
        else if (action == 2) {
            return APPLICATION_SEARCH_NEXT;
        }
        else if (action == 3) {
            return APPLICATION_PRINT_LAST;
        }
        else if (action == 4) {
            return APPLICATION_PRINT_SEARCHES;
        }
        else if (action == 5) {
            return APPLICATION_PRINT_SPECIFIC;
        }
        else if (action == 6) {
            return APPLICATION_RANK;
        }
        else {
            //invalid input
            std::cout << "\nINVALID ACTION" << std::endl;
        }
    }
    return APPLICATION_QUIT;
}

void takeAction(applicationActions a)
{
    /*
    Helper function to take the current action and implement that action

    Parameter a is a value of the applicationActions enum
    */

    if (a == APPLICATION_QUIT)
    {
        //quit application
        runApp = false;
    }
    else if (a == APPLICATION_SEARCH_NEW)
    {
        //ask for user search input
        unsigned int page = 0;
        std::string search;
        std::cout << "What would you like to search: ";
        std::cin >> search;

        //lowercase the search
        search = lowercase(search);

        for (auto const &i: searchlist)
        {  
            //if search has already been done, check the next page
            if (i.front() == search)
                page = page + 1;
        }
        
        //perform search and find result urls
        std::string data = searchGiphy(search, page);
        findItem(data, "url");
    }
    else if (a == APPLICATION_SEARCH_NEXT)
    {
        //if search results are empty, print message and move on
        if (searchlist.empty())
        {    
            system("clear");
            std::cout << "Can't use next without previous search.\n";
            std::cout << "Type something to move on. ";
            unsigned int t;
            std::cin >> t;
            std::cin.clear();
            std::cin.ignore();
        }
        else
        {
            //determine page number of search for previous element
            unsigned int page = 0;
            for (auto const &i: searchlist)
            {
                if (i.front() == searchlist.back().front())
                    page = page + 1;
            }

            //perform search based on previous search term 
            std::string data = searchGiphy(searchlist.back().front(), page);
            findItem(data, "url");
        }
    }
    else if (a == APPLICATION_PRINT_LAST)
    {
        //if search results are empty, print message and move on
        if (searchlist.empty())
        {    
            system("clear");
            std::cout << "Can't print last without previous search.\n";
            std::cout << "Type something to move on. ";
            unsigned int t;
            std::cin >> t;
            std::cin.clear();
            std::cin.ignore();
        }
        else
        {
            system("clear");
            
            //find page number of last search
            unsigned int page = 0;
            for (auto const &i: searchlist)
                if (i.front() == searchlist.back().front())
                    page = page + 1;
            
            //print last page search results
            std::cout << "Search: " << searchlist.back().front() << " Page: " << page << std::endl;
            for (auto const &i: searchlist.back())
            {
                if (i != searchlist.back().front())
                    std::cout << i << std::endl;
            }

            //take any input and move on
            std::cout << "-------------------------------------------------------\n";
            std::cout << "Type something to move on. ";
            unsigned int t;
            std::cin >> t;
            std::cin.clear();
            std::cin.ignore();
        }
    }
    else if (a == APPLICATION_PRINT_SEARCHES)
    {
        //if search results are empty, print message and move on
        if (searchlist.empty())
        {    
            system("clear");
            std::cout << "Can't print without searches.\n";
            std::cout << "Type something to move on. ";
            unsigned int t;
            std::cin >> t;
            std::cin.clear();
            std::cin.ignore();
        }
        else
        {
            //clear screen and print all results
            system("clear");
            printlist(searchlist);

            //take any input and move on
            std::cout << "-------------------------------------------------------\n";
            std::cout << "Type something to move on. ";
            unsigned int t;
            std::cin >> t;
            std::cin.clear();
            std::cin.ignore();
        }
    }
    else if (a == APPLICATION_PRINT_SPECIFIC)
    {
        //if search results are empty, print message and move on
        if (searchlist.empty())
        {    
            system("clear");
            std::cout << "Can't print without searches.\n";
            std::cout << "Type something to move on. ";
            unsigned int t;
            std::cin >> t;
            std::cin.clear();
            std::cin.ignore();
        }
        else
        {
            system("clear");
            std::cout << "Type previous search to print results.\n";

            //find and print list of all previous searches 
            std::list<std::string> printedresult;
            for (auto const &i: searchlist)
            {
                if (std::find(printedresult.begin(), printedresult.end(), i.front()) == printedresult.end())
                {
                    std::cout << i.front() << std::endl;
                    printedresult.insert(printedresult.end(), 1, i.front());
                }
            }

            //take user input for which search to print
            std::string search;
            std::cin >> search;
            search = lowercase(search);

            //if user input is not found in list of searches print invalid input and move on
            if (std::find(printedresult.begin(), printedresult.end(), search) == printedresult.end())
            {
                std::cout << "INVALID INPUT\n";
                std::cout << "Type something to move on. ";
                unsigned int t;
                std::cin >> t;
                std::cin.clear();
                std::cin.ignore();
            }
            else
            {
                //find total number of pages for search
                unsigned int page = 0;
                for (std::list< std::vector<std::string> >::iterator it = searchlist.begin(); it != searchlist.end(); ++it)
                {
                    if (search == (*it).front())
                        page = page + 1;
                }

                //take user input for which page to print
                std::cout << "Which page of results or all page results would you like to access?" << "(" << page << " pages)\n";
                std::cout << "Type the page number or 0 for all of the pages\n";
                unsigned int pagechoice;
                std::cin >> pagechoice;
                system("clear");
                
                //print all pages
                if (pagechoice == 0)
                {
                    std::cout << "Search: " << search << std::endl;

                    //iterate through all searches in order to print all urls for the specific search
                    for (std::list< std::vector<std::string> >::iterator it = searchlist.begin(); it != searchlist.end(); ++it)
                    {
                        if (search == (*it).front())
                        {
                            for (auto const &x: *it)
                            {
                                if (x != (*it).front())
                                {
                                    std::cout << x << std::endl;
                                }
                            }
                        }
                    }
                }
                else
                {
                    //counter variable to tick through pages until getting desired page choice
                    unsigned int counter = 0;

                    //loop through all searches
                    for (std::list< std::vector<std::string> >::iterator it = searchlist.begin(); it != searchlist.end(); ++it)
                    {
                        //find page with correct search
                        if (search == (*it).front())
                        {
                            counter = counter + 1;
                            //if correct page number print results
                            if (pagechoice == counter)
                            {
                                std::cout << "Search: " << search << " Page: " << pagechoice << std::endl;
                                for (auto const &x: *it)
                                {
                                    if (x != (*it).front())
                                    {
                                        std::cout << x << std::endl;
                                    }
                                }
                            }
                        }
                    }
                }
                //take any input and move on
                std::cout << "-------------------------------------------------------\n";
                std::cout << "Type something to move on. ";
                unsigned int t;
                std::cin >> t;
                std::cin.clear();
                std::cin.ignore();
                
            }
        }
    }
    else if (a == APPLICATION_RANK)
    {
        system("clear");

        //find all unique searches
        std::list<std::string> searches;
        for (std::list< std::vector<std::string> >::iterator it = searchlist.begin(); it != searchlist.end(); ++it)
        {
            if (std::find(searches.begin(), searches.end(), (*it).front()) == searches.end())
            {
                searches.insert(searches.end(), 1, (*it).front());
            }
        }

        std::cout << "List of Rank for Each Search.\n";

        //loop through all searches to find rank
        for (std::list<std::string>::iterator it = searches.begin(); it != searches.end(); ++it)
        {
            unsigned int rank = 0;
            for (std::list< std::vector<std::string> >::iterator iter = searchlist.begin(); iter != searchlist.end(); ++iter)
            {
                if ((*it) == (*iter).front())
                {
                    rank = rank + ((*iter).size() - 1);
                }
            }
            std::cout << (*it) << ": " << rank << std::endl;
        }

        //take any input and move on
        std::cout << "-------------------------------------------------------\n";
        std::cout << "Type something to move on. ";
        unsigned int t;
        std::cin >> t;
        std::cin.clear();
        std::cin.ignore();
    }


}

int main() {
    //application loop
    while (runApp)
    {
        applicationActions currentaction = userAction();
        takeAction(currentaction);
    }
}
