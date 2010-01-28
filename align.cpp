/* Given some standard input, fold it to the specified line length in the
 * same manner as the `fold` tool, but ensure that appropriate lines are
 * justified to the specified line length by inserting spaces at heuristically
 * appropriate places.
 */

#include <math.h>
#include <iostream>
#include <vector>
#include <boost/algorithm/string/trim.hpp>

using namespace std;


/**************************
 * HELPER FUNCTIONS FIRST *
 **************************/

void tokenize(
    const string& str,
    vector<string>& tokens,
    const string& delimiters = " "
    ) {
  /* Taken from the web.  Sorry.
   * Basically the same as strtok(), but with strings, not character arrays.
   */
  
  // Skip delimiters at beginning.
  string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos) {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
    }
  }

string replace(
    string input,
    string find,
    string repl,
    unsigned char max,
    bool from_end = false
    ) {
  /* Replace `max` occurrences of `find` with `repl` in `input`. */
  
  string out = input;
  if(from_end) {
    reverse(out.begin(), out.end());
    reverse(find.begin(), find.end());
    reverse(repl.begin(), repl.end());
    }
  
  size_t pos = 0;
  
  size_t find_len = find.length();
  size_t repl_len = repl.length();
  unsigned char replaced = 0;
  
  if( find_len != 0 ) {
    for(;
        (replaced < max) && 
        ((pos = out.find( find, pos )) != std::string::npos)
        ;) {
      out.replace( pos, find_len, repl );
      replaced++;
      pos += repl_len;
      }
    }
  
  if(from_end) {
    reverse(out.begin(), out.end());
    }
  
  return out;
  }


/*********************************
 * ALIGNMENT FUNCTIONS FOR SLUGS *
 *********************************/

// The four kinds of alignment you can do with this tool.
enum Alignment {
  align_left = 1,
  align_right,
  align_center,
  align_justify
  };

// Search-and-replace-ments for justification, in order of preference.
string justify_replacements[4][2] = {
  ". ", ".  ",
  "; ", ";  ",
  ", ", ",  ",
  " ",  "  "
  };
string justify_slug(string slug, unsigned char width, bool rtl) {
  //Given a literal slug of text, justify to the width.
  int padding = width - slug.length();
  
  int i = 0;
  int r;
  while(padding > 0) {
    int r = i % 4;
    slug = replace(slug, justify_replacements[r][0], justify_replacements[r][1], padding, rtl);
    padding = width - slug.length();
    i++;
    }
  
  return slug;
  }

string center_slug(string slug, unsigned char width) {
  int padding = width - slug.length();
  int left = floor(padding / 2);
  int right = padding - left;
  string centered_slug = "";
  for(int i = 0; i < left; i++) {
    centered_slug += " ";
    }
  centered_slug += slug;
  for(int i = 0; i < right; i++) {
    centered_slug += " ";
    }
  return centered_slug;
  }

string right_slug(string slug, unsigned char width) {
  int padding = width - slug.length();
  string righted_slug = "";
  for(int i = 0; i < padding; i++) {
    righted_slug += " ";
    }
  return righted_slug + slug;
  }

string align_para(string para, unsigned char width=72, Alignment alignment=align_justify) {
  // Given a paragraph, align it.
  string trimmed = boost::algorithm::trim_copy(para);
  
  // If this line shouldn't be justified ...
  if( trimmed.length() == 0 || 
      trimmed[0] == '|' ||
      trimmed[0] == '*' ||
      trimmed[0] == '-' ||
      trimmed[0] == '#'
      )
    {
      return para;
    }
  
  // Else, start justifying.
  vector<string> words;
  tokenize(trimmed, words, " ");
  
  vector<string> slugs(0);  // Holds our justified paragraph
  string current_slug = ""; // The current slug of text that will eventually be appended to `justified'
  
  for(int i = 0; i < words.size(); i++) {
    if( current_slug.length() + 1 + words[i].length()  <= width ) {
      current_slug += words[i]+" ";
      }
    else {
      current_slug = current_slug.substr(0,current_slug.length()-1);
      
      if(alignment == align_justify) {
        current_slug = justify_slug(current_slug, width, slugs.size() % 2);
        }
      else if(alignment == align_left) {
        // Nothing. We assume the input is aligned left.
        }
      else if(alignment == align_right) {
        current_slug = right_slug(current_slug, width);
        }
      else if(alignment == align_center) {
        current_slug = center_slug(current_slug, width);
        }
        
      slugs.push_back(current_slug); // + "\n";
      current_slug = words[i] + " ";
      }
    }
  
  // Add the final, shorter slug
  current_slug = current_slug.substr(0,current_slug.length()-1);
  if(alignment == align_justify || alignment == align_left) {
    // With justification, we don't do the last line.
    slugs.push_back(current_slug);
    }
  else if(alignment == align_right) {
    slugs.push_back(right_slug(current_slug, width));
    }
  else if(alignment == align_center) {
    slugs.push_back(center_slug(current_slug, width));
    }
  
  string justified_para = "";
  for(int i = 0; i < slugs.size(); i++) {
    justified_para += slugs[i];
    if (i < slugs.size()-1) {
      justified_para += "\n";
      }
    }
  
  return justified_para;
  }

int main(int argc, char* argv[]) {
  enum Alignment alignment = align_justify;
  int width = 72;
  
  for(int i = 1; i < argc; i++) {
    string arg = argv[i];
    if(arg == "left") {
      alignment = align_left;
      }
    else if(arg == "right") {
      alignment = align_right;
      }
    else if(arg == "center") {
      alignment = align_center;
      }
    else if(arg == "justify") {
      alignment = align_justify;
      }
    else {
      // Attempt to parse as line width
      const char *arg_char = arg.c_str();
      width = atoi(arg_char);
      if(!width) {
        // Not a valid argument.
        // Note that atoi() returning 0 on failure is OK;
        // a width of 0 doesn't make sense anyway.
        cout << "Please supply a valid argument.";
        return 1;
        }
      }
    }
  
  string line;
  bool first_line = true;
  while(cin) {
    if(!first_line) {
      cout << endl;
    }
    first_line = false;
    getline(cin, line);
    cout << align_para(line, width, alignment);
    }
  return 0;
  }
