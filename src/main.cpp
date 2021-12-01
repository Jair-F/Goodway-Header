#include <iostream>
#include <string>
#include <fstream>
#include <exception>
#include <windows.h>
#include <thread>

#define SEARCH_PHRASE "G21"
#define HEADER_FILE "./header.txt"
#define OUTPUT_FILE "output.txt"
#define PLACE_HOLDER_FACING_SPAHN "%FACING_SPAHN%"

int main() {
    HWND window = GetForegroundWindow();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    BringWindowToTop(window);                   // Bring window to top
    SetForegroundWindow(window);                // Then set it as foregroundwindow
    if (LockSetForegroundWindow(LSFW_LOCK)) {   // and then lock the foreground window
        std::cerr << "Failed to lock window!\n";
    }

    std::string input_file_name;
    std::cout << "Eingabedatei: ";
    std::getline(std::cin, input_file_name);

    LockSetForegroundWindow(LSFW_UNLOCK);   // Unlock the foreground window
    
    std::string facing_spahn;
    std::cout << "Facing-Spahn(mm): ";
    std::cin >> facing_spahn;
    for(char c : facing_spahn) { // check if the input is a digital number
        if (! ((c >= '0' && c <= '9') || c == '.')) {
            std::cerr << "Please enter a number!\n";
            return -1;
        }
    }
    

    // Remove the " if they are present
    if(input_file_name[0] == '"') {     
        input_file_name.erase(0, 1);
    }
    if(input_file_name[input_file_name.size()-1] == '"') {
        input_file_name.erase(input_file_name.size()-1, 1);
    }
    std::cout << input_file_name << '\n';
    


    std::fstream input_file;
    std::fstream header;
    std::fstream output_file;
    
    try {
        input_file.open(input_file_name, std::fstream::in);
        header.open(HEADER_FILE, std::fstream::in);
        output_file.open(OUTPUT_FILE, std::fstream::out | std::fstream::trunc);
        if(!input_file.good() || !header.good() || !output_file.good()) {
            throw std::runtime_error("Fehler beim oeffnen der Datei - ueberpruefe ob die Datei exisitiert oder die Berechtigungen");
        }
    }
    catch(std::runtime_error exc) {
        std::cerr << exc.what() << '\n';
    }
    catch(...) {
        std::cerr << "Unknown error\n";
    }



    std::string header_content;
    while(! header.eof()) {
        header_content += header.get();
    }
    header_content.pop_back(); // remove last element - eof
    //std::cout << header_content << '\n';
    header.close();
    
    {   // Searching facing_spahn
        unsigned int place_holder_begin = 0;
        while(header_content[place_holder_begin] != '%' && place_holder_begin < header_content.size()) {
            ++place_holder_begin;
        }
        
        unsigned int place_holder_end = place_holder_begin+1;
        while(header_content[place_holder_end] != '%' && place_holder_end < header_content.size()) {
            ++place_holder_end;
        }

        std::string place_holder = header_content.substr(place_holder_begin, place_holder_begin - place_holder_end);
        std::cout << place_holder << "\n\n";
        if(place_holder == PLACE_HOLDER_FACING_SPAHN) {
            header_content.erase(place_holder_begin, place_holder_end);
            std::cout << header_content.insert(place_holder_begin, facing_spahn) << '\n';
        }
        //std::cout << header_content << '\n';
    }



    std::string line;
    while(std::getline(input_file, line)) {
        output_file.write(line.c_str(), line.size());
        output_file.put('\n');  // write out a newline - the newline was removed at the getline
        
        if(line.find(SEARCH_PHRASE, 0) != line.npos) {
            output_file << header_content;
            if(! (header_content[header_content.size()-1] == '\n')) {   // add newline if there is no
                output_file.put('\n');
            }

            break;  // break out of the while loop - put the header only at the first occurence of SEARCH_PHRASE
        }
    }


    char buffer;
    while(input_file.read(&buffer, 1)) {
        output_file.put(buffer);
    }

    input_file.close();
    header.close();
    output_file.close();
    system((std::string("notepad.exe ") + OUTPUT_FILE).c_str());

    return 0;
}
