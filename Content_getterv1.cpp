// ContentExtractor.cpp
#include "Content_getter.h"
#include <queue>

namespace {
bool is_visible(const GumboNode* node) {
    if (node->type != GUMBO_NODE_ELEMENT) return false;
    GumboTag t = node->v.element.tag;
    return !(t == GUMBO_TAG_SCRIPT || t == GUMBO_TAG_STYLE ||
             t == GUMBO_TAG_META   || t == GUMBO_TAG_NOSCRIPT);
}
void dfs(const GumboNode* node, std::string& out) {
    if (node->type == GUMBO_NODE_TEXT) {
        out += node->v.text.text;
        out.push_back(' ');
    }
    if (node->type == GUMBO_NODE_ELEMENT && is_visible(node)) {
        const GumboVector* children = &node->v.element.children;
        for (unsigned i = 0; i < children->length; ++i)
            dfs(static_cast<GumboNode*>(children->data[i]), out);
    }
}
} // anonymous

std::pair<std::string,std::string> Content_getter::extract(const std::string& html) 
{
    GumboOutput* g = gumbo_parse(html.c_str());
    std::string title, body;
    // title
    GumboVector* roots = &g->root->v.element.children;
    for (unsigned i = 0; i < roots->length && title.empty(); ++i) {
        GumboNode* child = static_cast<GumboNode*>(roots->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT &&
            child->v.element.tag == GUMBO_TAG_HEAD) {
            const GumboVector* head_children = &child->v.element.children;
            for (unsigned j = 0; j < head_children->length; ++j) {
                GumboNode* n = static_cast<GumboNode*>(head_children->data[j]);
                if (n->type == GUMBO_NODE_ELEMENT &&
                    n->v.element.tag == GUMBO_TAG_TITLE &&
                    n->v.element.children.length) {
                    GumboNode* text = static_cast<GumboNode*>(
                        n->v.element.children.data[0]);
                    if (text->type == GUMBO_NODE_TEXT) title = text->v.text.text;
                }
            }
        }
    }
    // body – 简单 DFS
    dfs(g->root, body);
    gumbo_destroy_output(&kGumboDefaultOptions, g);
    return {title, body};
}
