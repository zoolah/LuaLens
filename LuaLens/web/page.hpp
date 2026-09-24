#pragma once
#include <string>
#include "web/css.hpp"
#include "web/disasm.hpp"
#include "web/graph.hpp"
#include "web/html.hpp"

namespace page {

inline std::string document() {
    std::string html = INDEX_HTML;
    auto head = html.find("</head>");
    if (head != std::string::npos)
        html.insert(head, std::string("<style>\n") + STYLE_CSS + "\n</style>\n");
    auto body = html.find("</body>");
    if (body != std::string::npos)
        html.insert(body, std::string("<script>\n") + DISASM_JS + "\n" + GRAPH_JS + "\n</script>\n");
    return html;
}

}
