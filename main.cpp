#include <chrono>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iomanip>

#include "nanodbc/nanodbc.h"

#include "scanner.hpp"

namespace po = boost::program_options;

long show( nanodbc::result& results )
{
  const short columns = results.columns();
  long rows_displayed = 0;

  std::cout << "\nDisplaying " << results.affected_rows() << " rows "
            << "(" << results.rowset_size() << " fetched at a time):" << std::endl;

  // show the column names
  for ( short i = 0; i < columns; ++i )
    std::cout << std::setw(20) << std::left << results.column_name( i );
  std::cout << std::endl;

  // show the column data for each row
  nanodbc::string const null_value = NANODBC_TEXT( "null" );
  while ( results.next() )
  {
    ++rows_displayed;
    for ( short col = 0; col < columns; ++col )
    {
      auto const value = results.get<nanodbc::string>( col, null_value );
      std::cout << std::setw(20) << std::left << value;
    }
    std::cout << std::endl;
  }

  return rows_displayed;
}

long just_fetch( nanodbc::result& results )
{
  long rows_displayed = 0;
  while ( results.next() )
  {
    rows_displayed++;
  }
  return rows_displayed;
}

#define TIME_MS( start_time )                                                                                     \
  std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - start_time ) \
      .count()

void execute_query( size_t line_number, const std::string& query_line, nanodbc::connection& conn, long fetch_size,
                    bool just_execute )
{
  nanodbc::statement statement;
  if ( boost::istarts_with( query_line, "SELECT" ) )
  {
    auto exec_time_start = std::chrono::high_resolution_clock::now();
    auto result = statement.execute_direct( conn, query_line, fetch_size );
    auto exec_time = TIME_MS( exec_time_start );
    auto fetch_time_start = std::chrono::high_resolution_clock::now();
    auto row_count = just_execute ? just_fetch( result ) : show( result );
    auto fetch_time = TIME_MS( fetch_time_start );
    std::cout << "Line" << line_number << ": Execute : " << exec_time << "ms, Fetch : " << fetch_time
              << "ms, RowCount : " << row_count << std::endl;
  }
  else
  {
    auto start_time = std::chrono::high_resolution_clock::now();
    statement.just_execute_direct( conn, query_line );
    std::cout << "Line" << line_number << ": Execute : " << TIME_MS( start_time )
              << "ms, affected_rows : " << statement.affected_rows() << std::endl;
  }
}

void replace_variables( std::string& input, std::unordered_map<std::string, std::string> variable_map )
{
  for ( const auto& elem : variable_map )
  {
    std::string key_regex = "$(" + elem.first + ")";
    boost::replace_all( input, key_regex, elem.second );
  }
}

void handle_options( const std::string& query_file_name, bool just_execute, long fetch_size )
{
  std::ifstream ifs( query_file_name );

  Scanner scanner( &ifs );
  MyState state{0};
  yy::Parser parser( scanner, state );
  parser.parse();


  std::shared_ptr<nanodbc::connection> connection;
  std::string connection_string;

  std::unordered_map<std::string, std::string> variable_map;

  std::string query_line;
  size_t line_number = 0;
  while ( std::getline( ifs, query_line ) )
  {
    ++line_number;
    boost::trim( query_line );
    if ( query_line.empty() || boost::starts_with( query_line, "--" ) )
      continue;

    const std::string SET = "DEFINE ";
    if ( boost::istarts_with( query_line, SET ) )
    {
      auto variable_str = query_line.substr( SET.length(), query_line.length() );
      boost::trim_left( variable_str );
      std::vector<std::string> strings;
      boost::split( strings, variable_str, boost::is_any_of( "=" ) );
      if ( strings.size() != 2 )
      {
        throw std::invalid_argument( std::string( "Variable declaration is wrong at line : " ) +
                                     std::to_string( line_number ) );
      }
      auto key = strings[0];
      auto value = strings[1];
      boost::trim_left( key );

      variable_map[key] = value;
      continue;
    }

    const std::string RESETFOLDER = "RESETFOLDER:";
    if ( boost::istarts_with( query_line, RESETFOLDER ) )
    {
      std::string folderpath = query_line.substr( RESETFOLDER.length(), query_line.length() );
      boost::trim( folderpath );
      replace_variables( folderpath, variable_map );
      if(boost::filesystem::is_directory(folderpath))
      {
        boost::filesystem::remove_all(folderpath);
      }

      boost::filesystem::create_directory(folderpath);
      continue;
    }

    const std::string CONNECT = "CONNECT:";
    if ( boost::istarts_with( query_line, CONNECT ) )
    {
      connection_string = query_line.substr( CONNECT.length(), query_line.length() );
      boost::trim( connection_string );
      replace_variables( connection_string, variable_map );
      connection = std::make_shared<nanodbc::connection>( connection_string );
      std::cout << "Connect is successful to database : " << connection->database_name() << std::endl;
      continue;
    }

    bool ignore_error = false;
    const std::string IGNOREERROR = "IGNOREERROR:";
    if ( boost::istarts_with( query_line, IGNOREERROR ) )
    {
      query_line = query_line.substr( IGNOREERROR.length(), query_line.length() );
      boost::trim( query_line );
      ignore_error = true;
    }

    if ( !connection )
    {
      throw std::invalid_argument( "No connection is established!" );
    }

    const std::string RECONNECT = "RECONNECT";
    if ( boost::istarts_with( query_line, RECONNECT ) )
    {
      connection->disconnect();
      connection = std::make_shared<nanodbc::connection>( connection_string );
      std::cout << "Reconnect is successful to database : " << connection->database_name() << std::endl;
      continue;
    }

    bool show_result = !just_execute;
    const std::string SHOWRESULT = "SHOWRESULT:";
    const std::string NORESULT = "NORESULT:";
    if ( boost::istarts_with( query_line, SHOWRESULT ) )
    {
      query_line = query_line.substr( SHOWRESULT.length(), query_line.length() );
      boost::trim( query_line );
      show_result = true;
    }
    else if ( boost::istarts_with( query_line, NORESULT ) )
    {
      query_line = query_line.substr( NORESULT.length(), query_line.length() );
      boost::trim( query_line );
      show_result = false;
    }

    replace_variables( query_line, variable_map );

    if ( ignore_error )
    {
      try
      {
        execute_query( line_number, query_line, *connection, fetch_size, !show_result );
      }
      catch ( std::runtime_error const& e )
      {
        std::cerr << "Error occurred : " << e.what() << std::endl;
      }
    }
    else
    {
      execute_query( line_number, query_line, *connection, fetch_size, !show_result );
    }
  }
}

int main( int argc, char** argv )
{
  try
  {
    po::variables_map option_map;
    po::options_description desc( "Options" );
    desc.add_options()
      ( "query_file,f", po::value<std::string>()->required(), "query file path to be executed." )
      ( "just_execute,j", "just execute, don't show the results" )
      ( "fetch_size,s", po::value<long>()->default_value( 100 ), "fetch size while select" )
      ( "help,h", "SQL FILE OPTIONS:\n\nCONNECT: takes a connection string and connects.\n\n"
        "RECONNECT disconnects existing connection and reconnect by using last connection string. It is useful in order to clear cashing etc on client side.\n\n"
        "IGNOREERROR: continues to execute further queries even if this query is failed.\n\n"
        "SHOWRESULT: display results of query. Overwrites just_execute program option.\n\n"
        "NORESULT: do not display results of query. Overwrites just_execute program option.\n\n"
        "SET <key>=<value> variable declaration. Then variables can be used as such: $(<key>)\n\n"
        "RESETFOLDER:<folder/path> tries to delete and recreate given folder\n\n" );

    po::store( po::command_line_parser( argc, argv ).options( desc ).allow_unregistered().run(), option_map );

    if ( option_map.count( "help" ) )
    {
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

    po::notify( option_map );

    auto query_file_name = option_map["query_file"].as<std::string>();
    bool just_execute = option_map.count( "just_execute" ) > 0;
    auto fetch_size = option_map["fetch_size"].as<long>();

    handle_options( query_file_name, just_execute, fetch_size );
  }
  catch ( std::exception& e )
  {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
  catch ( ... )
  {
    std::cerr << "Exception of unknown type!\n";
  }

  return 0;
}
