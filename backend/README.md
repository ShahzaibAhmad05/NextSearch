## To Set up the Backend locally

- Create a folder in the backend with the following structure:

````
third_party/
    nlohmann/
        json.hpp
    httplib.h
````

- Copy the code for json.hpp from [here](https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp).

- Similarly, copy the code for httplib.h from [here](https://github.com/yhirose/cpp-httplib/blob/master/httplib.h).

- Make sure your terminal is in the backend directory, for example, it should look like this (on windows):

````
PS C:/Users/<username>/Projects/NextSearch/backend>
````

- Run the following command to build the executables:

````
cmake -S . -B build
cmake --build build
````

and Done! Now the backend is ready for testing.
