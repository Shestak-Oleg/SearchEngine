#include <iostream>
#include "converter.h"
#include "index.h"
#include "search.h"

int main() {
    try {
        ConverterJSON converter;
        InvertedIndex index;
        index.updateDocumentBase(converter.getTextDocuments());
        std::cout << converter.getName() << " [version " << converter.getVersion() << "] starting\n";
        SearchServer server(index);
        converter.putAnswers(server.search(converter.getRequests(), converter.getResponsesLimit()));
        std::cout << converter.getName() << " [version " << converter.getVersion() << "] has completed successfully\n";
        return 0;
    } catch (const std::exception& ex) {
        // Обработка стандартных исключений
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    } catch (...) {
        // Обработка всех остальных исключений
        std::cerr << "An unknown error occurred. Exit!\n";
        return 1;
    }
}
