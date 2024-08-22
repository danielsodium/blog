
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <vector>

namespace fs = std::filesystem;

std::string escapeHtml(const std::string &text) {
    std::string escaped = text;
    escaped = std::regex_replace(escaped, std::regex("&"), "&amp;");
    escaped = std::regex_replace(escaped, std::regex("<"), "&lt;");
    escaped = std::regex_replace(escaped, std::regex(">"), "&gt;");
    escaped = std::regex_replace(escaped, std::regex("\""), "&quot;");
    escaped = std::regex_replace(escaped, std::regex("'"), "&#39;");
    return escaped;
}

std::string markdownToHtml(const std::string &markdown) {
    std::istringstream stream(markdown);
    std::string line;
    std::string html;

    std::regex imageRegex(R"(\!\[([^\]]*)\]\(([^\)]+)\))");
    std::regex linkRegex(R"(\[([^\]]+)\]\(([^\)]+)\))");

    while (std::getline(stream, line)) {
        if (line.empty()) {
            html += "<p></p>\n";
        } else if (line.rfind("###### ", 0) == 0) {
            html += "<h6>" + line.substr(7) + "</h6>\n";
        } else if (line.rfind("##### ", 0) == 0) {
            html += "<h5>" + line.substr(6) + "</h5>\n";
        } else if (line.rfind("#### ", 0) == 0) {
            html += "<h4>" + line.substr(5) + "</h4>\n";
        } else if (line.rfind("### ", 0) == 0) {
            html += "<h3>" + line.substr(4) + "</h3>\n";
        } else if (line.rfind("## ", 0) == 0) {
            html += "<h2>" + line.substr(3) + "</h2>\n";
        } else if (line.rfind("# ", 0) == 0) {
            html += "<h1>" + line.substr(2) + "</h1>\n";
        } else if (line.rfind("```", 0) == 0) {
            html += "<pre><code>";
            while (std::getline(stream, line) && line != "```") {
                html += escapeHtml(line) + "\n";
            }
            html += "</code></pre>\n";
        } else if (line.rfind("- ", 0) == 0) {
            html += "<ul>\n";
            do {
                html += "<li>" + line.substr(2) + "</li>\n";
            } while (std::getline(stream, line) && line.rfind("- ", 0) == 0);
            html += "</ul>\n";
            continue; // Prevent extra <p> from being added
        } else {
            // Replace image markdown with HTML
            line = std::regex_replace(line, imageRegex, "<img src=\"$2\" alt=\"$1\">");
            // Replace link markdown with HTML
            line = std::regex_replace(line, linkRegex, "<a href=\"$2\">$1</a>");
            html += "<p>" + line + "</p>\n";
        }
    }

    return html;
}

void generateHtmlFile(const std::string &templateFile, const std::string &markdownFile, const std::string &htmlFile) {
    // Load the markdown file
    std::ifstream mdFile(markdownFile);
    if (!mdFile.is_open()) {
        std::cerr << "Could not open markdown file: " << markdownFile << std::endl;
        return;
    }
    std::stringstream mdContent;
    mdContent << mdFile.rdbuf();
    mdFile.close();

    // Convert markdown to HTML
    std::string htmlContent = markdownToHtml(mdContent.str());

    // Load the HTML template
    std::ifstream tplFile(templateFile);
    if (!tplFile.is_open()) {
        std::cerr << "Could not open template file: " << templateFile << std::endl;
        return;
    }
    std::stringstream tplContent;
    tplContent << tplFile.rdbuf();
    tplFile.close();

    // Replace <BlogPost></BlogPost> in the template with the converted HTML content
    std::string outputHtml = std::regex_replace(tplContent.str(), std::regex(R"(<BlogPost></BlogPost>)"), htmlContent);

    // Create the HTML file
    std::ofstream outFile(htmlFile);
    if (!outFile.is_open()) {
        std::cerr << "Could not create HTML file: " << htmlFile << std::endl;
        return;
    }

    // Write the generated HTML content to the file
    outFile << outputHtml;
    outFile.close();
    std::cout << "HTML file generated successfully: " << htmlFile << std::endl;
}

// Function to generate the blog page with links to all the HTML files
void generateBlogPage(const std::string &templateFile, const std::vector<std::string> &htmlFiles, const std::string &outputFile) {
    // Load the HTML template
    std::ifstream tplFile(templateFile);
    if (!tplFile.is_open()) {
        std::cerr << "Could not open template file: " << templateFile << std::endl;
        return;
    }
    std::stringstream tplContent;
    tplContent << tplFile.rdbuf();
    tplFile.close();

    // Generate the list of links to HTML files
    std::string listContent = "<ul>\n";
    for (const auto &htmlFile : htmlFiles) {
        listContent += "<li><a href=\"" + htmlFile + "\">" + htmlFile + "</a></li>\n";
    }
    listContent += "</ul>\n";

    // Replace <BlogList></BlogList> in the template with the generated list
    std::string outputHtml = std::regex_replace(tplContent.str(), std::regex(R"(<BlogList></BlogList>)"), listContent);

    // Create the blog page HTML file
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Could not create blog page file: " << outputFile << std::endl;
        return;
    }

    // Write the generated HTML content to the file
    outFile << outputHtml;
    outFile.close();
    std::cout << "Blog page generated successfully: " << outputFile << std::endl;
}

int main() {
    std::string pagesDir = "./pages";
    std::string outDir = "./out";
    std::string templateFile = "./templates/post.html";
    std::string cssFileSrc = "./templates/styles.css";
    std::string cssFileDest = outDir + "/styles.css";
    std::string blogTemplateFile = "./templates/blog.html";
    std::string blogOutputFile = outDir + "/blog.html";

    // Create the output directory if it doesn't exist
    if (!fs::exists(outDir)) {
        fs::create_directory(outDir);
    }

    // Copy the CSS file to the output directory
    if (std::system(("cp " + cssFileSrc + " " + cssFileDest).c_str()) != 0) {
        std::cerr << "Failed to copy CSS file to " << cssFileDest << std::endl;
        return 1;
    }

    // Vector to store the paths of generated HTML files
    std::vector<std::string> htmlFiles;

    // Iterate over all markdown files in the pages directory
    for (const auto &entry : fs::directory_iterator(pagesDir)) {
        if (entry.path().extension() == ".md") {
            std::string markdownFile = entry.path().string();
            std::string htmlFile = outDir + "/" + entry.path().stem().string() + ".html";

            // Generate the HTML file
            generateHtmlFile(templateFile, markdownFile, htmlFile);

            // Store the relative path of the generated HTML file
            htmlFiles.push_back(entry.path().stem().string() + ".html");
        }
        else {
            std::system(("cp " + entry.path().string() + " " + outDir + "/" + entry.path().filename().string()).c_str());
        }
    }

    // Generate the blog page with the list of links to all generated HTML files
    generateBlogPage(blogTemplateFile, htmlFiles, blogOutputFile);

    std::cout << "All markdown files have been converted to HTML and a blog page has been generated." << std::endl;
    return 0;
}

