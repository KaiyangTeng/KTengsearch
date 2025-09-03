// Enhanced Content_getter.cpp
#include "Content_getter.h"
#include <queue>
#include <regex>
#include <sstream>

namespace {

// 过滤不可见或杂质标签
bool is_visible(const GumboNode* node) {
    if (node->type != GUMBO_NODE_ELEMENT) return false;
    GumboTag t = node->v.element.tag;
    return !(t == GUMBO_TAG_SCRIPT || t == GUMBO_TAG_STYLE ||
             t == GUMBO_TAG_META   || t == GUMBO_TAG_NOSCRIPT ||
             t == GUMBO_TAG_NAV    || t == GUMBO_TAG_HEADER  ||
             t == GUMBO_TAG_FOOTER || t == GUMBO_TAG_ASIDE   ||
             t == GUMBO_TAG_SVG    || t == GUMBO_TAG_FORM);
}

// 判断字符串是否包含实质性内容
bool is_meaningful_text(const std::string& text) {
    for (char c : text) {
        if (!isspace(c)) return true;
    }
    return false;
}




void dfs(const GumboNode* node, std::string& out) 
{
    if(out.size()>400) return;
    if (node->type == GUMBO_NODE_TEXT) {
        std::string text = node->v.text.text;
        if (is_meaningful_text(text)) {
            out += text + " ";
            if(out.size()>400) return;
        }
    }
    if (node->type == GUMBO_NODE_ELEMENT && is_visible(node)) {
        const GumboVector* children = &node->v.element.children;
        for (unsigned i = 0; i < children->length; ++i) {
            dfs(static_cast<GumboNode*>(children->data[i]), out);
            if(out.size()>400) return;
        }
    }
}





// 压缩多余空格
std::string clean_whitespace(const std::string& text) {
    std::regex ws_re("\\s+");
    return std::regex_replace(text, ws_re, " ");
}

} // namespace

std::pair<std::string, std::string> Content_getter::extract(const std::string& html) 
{
    GumboOutput* g = gumbo_parse(html.c_str());
    std::string title, body;

    // 提取标题
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
                    if (text->type == GUMBO_NODE_TEXT)
                        title = text->v.text.text;
                }
            }
        }
    }

    // 提取正文内容
    dfs(g->root, body);
    body = clean_whitespace(body);  // 清理多余空格

    gumbo_destroy_output(&kGumboDefaultOptions, g);
    return {title, body};
}

















