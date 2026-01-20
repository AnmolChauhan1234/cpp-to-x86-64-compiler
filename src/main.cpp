#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "./tokenization.hpp"
#include "./parser.hpp"
#include "./generator.hpp"

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        std::cout << "Wrong input format the input should be ./mycomiper <input file>";
        return EXIT_FAILURE;
    }

    std::string contents;

    {
        std::ifstream file(argv[1]);
        if (!file)
        {
            std::cerr << "Error: could not open file " << argv[1] << std::endl;
            return EXIT_FAILURE;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        contents = buffer.str();
    }

    // std::cout << "File contents:\n" << contents << std::endl;

    Tokeniser tokeniser(std::move(contents));
    std::vector<Token> tokens = tokeniser.tokenise();

    Parser parser(std::move(tokens));

    NodeProg prog = parser.parse();

    Generator generator(std::move(prog));
    std::string output = generator.gen_prog();

    // std::cout<<output<<std::endl;

    {
        std::fstream file("out.asm", std::ios::out);
        file << output;
    }
    system("nasm -felf64 print.asm -o print.o");
    system("nasm -felf64 errors.asm -o errors.o");
    system("nasm -felf64 out.asm -o out.o");
    system("ld -o out out.o print.o errors.o");
    return EXIT_SUCCESS;
}