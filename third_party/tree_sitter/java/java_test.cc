#include "third_party/tree_sitter/java/java.h"
#include "third_party/tree_sitter/parser.h"

#include <cstddef>
#include <string>
#include <gtest/gtest.h>
#include "absl/strings/string_view.h"

TEST(TreeSitterJavaTest, CanGetLanguage) {
  const auto *language = tree_sitter_java();
  EXPECT_EQ(language->version, 13);
}

TEST(TreeSitterJavaTest, ParsesJavaCode) {
  static const absl::string_view source = R"(package com.gravypod.sample;

public class Main {
  public static void main(String[] args) {
    int a = 1;
    int b = 2;
    if (a < b) {
      System.exit(0);
    }
  }
})";
  auto *parser = ts_parser_new();
  EXPECT_TRUE(ts_parser_set_language(parser, tree_sitter_java()));

  auto *tree = ts_parser_parse_string(
    parser,
    NULL,
    source.data(), source.size()
  );

  auto root = ts_tree_root_node(tree);

  const char *sexpr = ts_node_string(root);
  std::string generated(sexpr);
  delete sexpr;
  EXPECT_EQ(generated,
    "(program "                                                                   //
        "(package_declaration "                                                   //
            "(scoped_identifier "                                                 //
                "scope: (scoped_identifier "                                      //
                    "scope: (identifier) "                                        //
                    "name: (identifier)) "                                        //
                "name: (identifier))) "                                           //
        "(class_declaration (modifiers) "                                         //
            "name: (identifier) "                                                 //
            "body: (class_body "                                                  //
                "(method_declaration (modifiers) "                                //
                    "type: (void_type) "                                          //
                    "name: (identifier) "                                         //
                    "parameters: (formal_parameters "                             //
                        "(formal_parameter "                                      //
                            "type: (array_type "                                  //
                                "element: (type_identifier) "                     //
                                "dimensions: (dimensions)) "                      //
                            "name: (identifier))) "                               //
                    "body: (block "                                               //
                        "(local_variable_declaration "                            //
                            "type: (integral_type) "                              //
                            "declarator: (variable_declarator "                   //
                                "name: (identifier) "                             //
                                "value: (decimal_integer_literal))) "             //
                        "(local_variable_declaration "                            //
                            "type: (integral_type) "                              //
                            "declarator: (variable_declarator "                   //
                                "name: (identifier) "                             //
                                "value: (decimal_integer_literal))) "             //
                        "(if_statement "                                          //
                            "condition: (parenthesized_expression "               //
                                "(binary_expression "                             //
                                    "left: (identifier) "                         //
                                    "right: (identifier))) "                      //
                            "consequence: (block "                                //
                                "(expression_statement "                          //
                                    "(method_invocation "                         //
                                        "object: (identifier) "                   //
                                        "name: (identifier) "                     //
                                        "arguments: (argument_list "              //
                                            "(decimal_integer_literal)))))))))))" //
  );

}

