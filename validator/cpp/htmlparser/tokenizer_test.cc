#include "tokenizer.h"

#include "gtest/gtest.h"
#include "token.h"

// TODO: Add more complex html not-well-formed documents to cover
// complex tokenization scenarios. Eg. <html>><head><<body;>> etc..

TEST(TokenizerTest, BasicTokenizationOfADocument) {
  std::string html =
      "<html><head><title>hello</title></head><body><div>Hello</div>"
      "<textarea id=\"my-text\" class=\"my-style\"></textarea>"
      "<img src=\"foo.png\" /></body></html>";

  htmlparser::Tokenizer t(html);

  std::vector<htmlparser::Token> tokens;
  while (!t.IsEOF()) {
    htmlparser::TokenType tt = t.Next();
    if (tt == htmlparser::TokenType::ERROR_TOKEN) break;
    htmlparser::Token token = t.token();
    tokens.push_back(token);
  }

  EXPECT_EQ(tokens.size(), 15) << "Total 15 tokens generated by tokenizer";

  // First three start tags. <html><head><title>.
  EXPECT_EQ(tokens[0].token_type, htmlparser::TokenType::START_TAG_TOKEN)
      << "<html> start tag 0";
  EXPECT_EQ(tokens[1].token_type, htmlparser::TokenType::START_TAG_TOKEN)
      << "<head> start tag 1";
  EXPECT_EQ(tokens[2].token_type, htmlparser::TokenType::START_TAG_TOKEN)
      << "<title> start tag 2";

  // Text "hello".
  EXPECT_EQ(tokens[3].token_type , htmlparser::TokenType::TEXT_TOKEN)
      << "Text hello in title tag 3";

  // End tags. </title></head>.
  EXPECT_EQ(tokens[4].token_type , htmlparser::TokenType::END_TAG_TOKEN)
      << "</title> end tag 4";
  EXPECT_EQ(tokens[5].token_type , htmlparser::TokenType::END_TAG_TOKEN)
      << "</head end tag 5";

  // Start tags. <body><div>
  EXPECT_EQ(tokens[6].token_type , htmlparser::TokenType::START_TAG_TOKEN)
      << "<body> start tag 6";
  EXPECT_EQ(tokens[7].token_type , htmlparser::TokenType::START_TAG_TOKEN)
      << "<div> start tag 6";

  // Text "Hello"
  EXPECT_EQ(tokens[8].token_type , htmlparser::TokenType::TEXT_TOKEN)
      << "Hello text token 8";
  EXPECT_EQ(tokens[8].data , "Hello") << "Hello string inside <div> 8";

  // End div.
  EXPECT_EQ(tokens[9].token_type , htmlparser::TokenType::END_TAG_TOKEN)
      << "</div> end tag 9";

  // Start textarea.
  EXPECT_EQ(tokens[10].token_type , htmlparser::TokenType::START_TAG_TOKEN)
      << "<textarea> start tag 10";

  // Two attributes in <textarea>
  EXPECT_EQ(tokens[10].attributes.size() , 2)
      << "<textarea 2 attributes.";

  EXPECT_EQ(tokens[10].attributes[0].key , "id")
      << "textarea first attribute is id";
  EXPECT_EQ(tokens[10].attributes[0].value , "my-text")
      << "textarea first attribute id value is my-text";
  EXPECT_EQ(tokens[10].attributes[1].key , "class")
      << "textarea second attribute is class";
  EXPECT_EQ(tokens[10].attributes[1].value , "my-style")
      << "textarea second attribute class value is my-style";

  // End textarea.
  EXPECT_EQ(tokens[11].token_type , htmlparser::TokenType::END_TAG_TOKEN)
      << "<textarea> end tag 11";

  // img tag. self closing.
  EXPECT_EQ(tokens[12].token_type ,
            htmlparser::TokenType::SELF_CLOSING_TAG_TOKEN)
      << "<img> self closing tag 12";
  EXPECT_EQ(tokens[12].attributes.size() , 1)
      << "img only one attribute";
  EXPECT_EQ(tokens[12].attributes[0].key , "src")
      << "img first attribute is src";
  EXPECT_EQ(tokens[12].attributes[0].value , "foo.png")
      << "img first attribute src value is foo.png";

  // Close body, html.
  EXPECT_EQ(tokens[13].token_type , htmlparser::TokenType::END_TAG_TOKEN)
      << "body close tag 13";
  EXPECT_EQ(tokens[14].token_type , htmlparser::TokenType::END_TAG_TOKEN)
      << "html close tag 14";

  EXPECT_EQ(t.LinesProcessed(), 1);

  html = R"HTML(
<html>
  <head>
    <title>hello</title>
  </head>
  <body>
    <div>Hello</div>
    <textarea id="my-text" class="my-style"></textarea>
    <img src="foo.png" />
  </body>
</html>)HTML";
  //  ^--------- 7th column.

  htmlparser::Tokenizer t2(html);

  tokens.clear();
  while (!t2.IsEOF()) {
    htmlparser::TokenType tt = t2.Next();
    if (tt == htmlparser::TokenType::ERROR_TOKEN) break;
    htmlparser::Token token = t2.token();
    if (tt == htmlparser::TokenType::START_TAG_TOKEN) {
      switch (token.atom) {
        case htmlparser::Atom::TITLE:
          EXPECT_EQ(token.line_col_in_html_src.first, 4);
          EXPECT_EQ(token.line_col_in_html_src.second, 5);
          break;
        case htmlparser::Atom::BODY:
          EXPECT_EQ(token.line_col_in_html_src.first, 6);
          EXPECT_EQ(token.line_col_in_html_src.second, 3);
          break;
        case htmlparser::Atom::IMG:
          EXPECT_EQ(token.line_col_in_html_src.first, 9);
          EXPECT_EQ(token.line_col_in_html_src.second, 5);
          break;
        default:
          break;
      }
    }
    tokens.push_back(token);
  }

  EXPECT_EQ(t2.LinesProcessed(), 11);
  EXPECT_EQ(t2.CurrentPosition().first, 11 /* </html> line */);
  EXPECT_EQ(t2.CurrentPosition().second, 7 /* end of </html> */);
  EXPECT_EQ(tokens.size(), 25 /* 15 + 10 new lines */);

  html = R"HTML(
<html>
  <head>
    <title>hello</title>
  </head>
  <body>
    <div>Hello</div>
    <textarea id="my-text" class="my-style"></textarea>
    <h1>foo</h1><h2>bar</h2><img src="foo.png" /></body></html>)HTML";
  htmlparser::Tokenizer t3(html);

  tokens.clear();
  while (!t3.IsEOF()) {
    htmlparser::TokenType tt = t3.Next();
    if (tt == htmlparser::TokenType::ERROR_TOKEN) break;
    htmlparser::Token token = t3.token();
    tokens.push_back(token);
  }

  EXPECT_EQ(t3.LinesProcessed(), 9);
  EXPECT_EQ(t3.CurrentPosition().first, 9 /* </html> line */);
  EXPECT_EQ(t3.CurrentPosition().second, 63 /* end of </html> */);
  EXPECT_EQ(tokens.size(), 29 /* 21 + 8 new lines */);

  // Multiple tags (especially raw text tags (textarea, noscript, style) on the
  // same line.
  html = R"HTML(
<html>
  <head>
    <title>hello</title>
  </head>
  <body>
  <div id="one">helloworld</div><div id="two">anotherdivonsameline</div>
  <style id="first">.foo{color:red;}</style><noscript><style id="second">.foo{color:black;}</style></noscript>
  </body>
</html>)HTML";

  htmlparser::Tokenizer t4(html);
  tokens.clear();
  while (!t4.IsEOF()) {
    htmlparser::TokenType tt = t4.Next();
    if (tt == htmlparser::TokenType::ERROR_TOKEN) break;
    htmlparser::Token token = t4.token();
    if (tt == htmlparser::TokenType::START_TAG_TOKEN) {
      tokens.push_back(token);
      switch (token.atom) {
        case htmlparser::Atom::DIV:
          if (token.attributes.at(0).value == "one") {
            EXPECT_EQ(token.line_col_in_html_src.first, 7);
            EXPECT_EQ(token.line_col_in_html_src.second, 3);
          } else {
            EXPECT_EQ(token.line_col_in_html_src.first, 7);
            EXPECT_EQ(token.line_col_in_html_src.second, 33);
          }
          break;
        case htmlparser::Atom::STYLE:
          if (token.attributes.at(0).value == "first") {
            EXPECT_EQ(token.line_col_in_html_src.first, 8);
            EXPECT_EQ(token.line_col_in_html_src.second, 3);
          } else {
            EXPECT_EQ(token.line_col_in_html_src.first, 8);
            EXPECT_EQ(token.line_col_in_html_src.second, 55);
          }
          break;
        case htmlparser::Atom::NOSCRIPT:
          EXPECT_EQ(token.line_col_in_html_src.first, 8);
          EXPECT_EQ(token.line_col_in_html_src.second, 45);
          break;
        default:
          break;
      }
    }
  }
  EXPECT_EQ(tokens.size(), 8);

  // TODO: DO NOT ADD MORE TESTS HERE. Split all above tests in
  // their respective test cases.
}

TEST(TokenizerTest, TestMustangTemplateCase) {
  std::string_view template_html = R"HTML(<html>
<head></head>
<body>
<template type="amp-mustache">
<p {{#bluetheme}}class=foo{{/bluetheme}}>
<script {{#fastrender}}async{{/fastrender}} src="big.js"></script>
<div data-{{variable}}="hello">hello world</div>
<span data-{{\notallowed}}="hello">hi world</span>
<h1 data-{{/notallowed}}="hello">hi world</h1>
<h2 data-{{#allowed}}foo{{/allowed}}="hello">hi world</h1>
<img {{#border}}class=border src=foo.png>
</template>
</body>
</html>)HTML";

  std::vector<htmlparser::Token> tokens;
  htmlparser::Tokenizer t(template_html);
  while (!t.IsEOF()) {
    htmlparser::TokenType tt = t.Next(true);
    if (tt == htmlparser::TokenType::ERROR_TOKEN) break;
    htmlparser::Token token = t.token();
    if (tt == htmlparser::TokenType::START_TAG_TOKEN) {
      tokens.push_back(token);
      switch (token.atom) {
        case htmlparser::Atom::TEMPLATE:
          EXPECT_EQ(token.attributes.size(), 1);
          EXPECT_EQ(token.attributes.at(0).key, "type");
          EXPECT_EQ(token.attributes.at(0).value, "amp-mustache");
          break;
        case htmlparser::Atom::P:
          EXPECT_EQ(token.attributes.size(), 1);
          EXPECT_EQ(token.attributes.at(0).key,
                    "{{#bluetheme}}class");
          EXPECT_EQ(token.attributes.at(0).value, "foo{{/bluetheme}}");
          break;
        case htmlparser::Atom::SCRIPT:
          EXPECT_EQ(token.attributes.size(), 2);
          EXPECT_EQ(token.attributes.at(0).key,
                    "{{#fastrender}}async{{/fastrender}}");
          EXPECT_EQ(token.attributes.at(0).value, "");
          EXPECT_EQ(token.attributes.at(1).key, "src");
          EXPECT_EQ(token.attributes.at(1).value, "big.js");
          break;
        case htmlparser::Atom::DIV:
          EXPECT_EQ(token.attributes.size(), 1);
          EXPECT_EQ(token.attributes.at(0).key, "data-{{variable}}");
          EXPECT_EQ(token.attributes.at(0).value, "hello");
          break;
        case htmlparser::Atom::SPAN:
          EXPECT_EQ(token.attributes.size(), 1);
          EXPECT_EQ(token.attributes.at(0).key, "data-{{\\notallowed}}");
          EXPECT_EQ(token.attributes.at(0).value, "hello");
          break;
        case htmlparser::Atom::H1:
          EXPECT_EQ(token.attributes.size(), 2);
          EXPECT_EQ(token.attributes.at(0).key, "data-{{");
          EXPECT_EQ(token.attributes.at(0).value, "");
          EXPECT_EQ(token.attributes.at(1).key, "notallowed}}");
          EXPECT_EQ(token.attributes.at(1).value, "hello");
          break;
        case htmlparser::Atom::H2:
          EXPECT_EQ(token.attributes.size(), 1);
          EXPECT_EQ(token.attributes.at(0).key,
                    "data-{{#allowed}}foo{{/allowed}}");
          EXPECT_EQ(token.attributes.at(0).value, "hello");
          break;
        case htmlparser::Atom::IMG:
          EXPECT_EQ(token.attributes.size(), 2);
          EXPECT_EQ(token.attributes.at(0).key, "{{#border}}class");
          EXPECT_EQ(token.attributes.at(0).value, "border");
          EXPECT_EQ(token.attributes.at(1).key, "src");
          EXPECT_EQ(token.attributes.at(1).value, "foo.png");
          break;
        default:
          break;
      }
    }
  }
  EXPECT_EQ(tokens.size(), 11);

  // Tokenization differs slightly in non-template mode.
  tokens.clear();
  htmlparser::Tokenizer t2(template_html);
  while (!t2.IsEOF()) {
    htmlparser::TokenType tt = t2.Next(false);
    if (tt == htmlparser::TokenType::ERROR_TOKEN) break;
    htmlparser::Token token = t2.token();
    if (tt == htmlparser::TokenType::START_TAG_TOKEN) {
      tokens.push_back(token);
      switch (token.atom) {
        case htmlparser::Atom::TEMPLATE:
          EXPECT_EQ(token.attributes.size(), 1);
          EXPECT_EQ(token.attributes.at(0).key, "type");
          EXPECT_EQ(token.attributes.at(0).value, "amp-mustache");
          break;
        case htmlparser::Atom::P:
          EXPECT_EQ(token.attributes.size(), 1);
          EXPECT_EQ(token.attributes.at(0).key,
                    "{{#bluetheme}}class");
          EXPECT_EQ(token.attributes.at(0).value, "foo{{/bluetheme}}");
          break;
        case htmlparser::Atom::SCRIPT:
          EXPECT_EQ(token.attributes.size(), 3);  // <-- template mode 2.
          EXPECT_EQ(token.attributes.at(0).key,
                    "{{#fastrender}}async{{");  // template mode entire block.
          EXPECT_EQ(token.attributes.at(0).value, "");
          EXPECT_EQ(token.attributes.at(1).key, "fastrender}}");
          EXPECT_EQ(token.attributes.at(1).value, "");
          EXPECT_EQ(token.attributes.at(2).key, "src");
          EXPECT_EQ(token.attributes.at(2).value, "big.js");
          break;
        case htmlparser::Atom::DIV:
          EXPECT_EQ(token.attributes.size(), 1);
          EXPECT_EQ(token.attributes.at(0).key, "data-{{variable}}");
          EXPECT_EQ(token.attributes.at(0).value, "hello");
          break;
        case htmlparser::Atom::IMG:
          EXPECT_EQ(token.attributes.size(), 2);
          EXPECT_EQ(token.attributes.at(0).key, "{{#border}}class");
          EXPECT_EQ(token.attributes.at(0).value, "border");
          EXPECT_EQ(token.attributes.at(1).key, "src");
          EXPECT_EQ(token.attributes.at(1).value, "foo.png");
          break;
        default:
          break;
      }
    }
  }
  EXPECT_EQ(tokens.size(), 11);
}
