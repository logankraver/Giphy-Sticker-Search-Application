# Giphy-Sticker-Search-Application
This application is an interactive C++ console app that searches for stickers from giphy given user input. The application uses boost.asio in order to perform the searches and makes request to the giphy api. The data is stored in STL templates for lists and vectors, as all of the searches are stored in a global list as vectors containing the search term and links. Each vector represents a page of 20 results.
To run the program, just compile the cpp file with a link to boost.asio and parameter -lpthread for multithreading.
