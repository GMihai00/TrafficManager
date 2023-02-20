// JUST A SAMPLE OF CODE FOR DB
#include "utile/DBWrapper.hpp"
#include <exception>

using namespace std;


int main()
{
	//sql::Driver *driver;
	//sql::Connection* con;
	//sql::Statement* stmt;
	//sql::PreparedStatement* pstmt;
	try
	{
		utile::DBWrapper dbwrapper;
		std::cout << "CONNECTED";
	}
	catch (std::runtime_error& ec)
	{
		std::cout << "Failed to establish connection error: " << ec.what();
	}

	/*
	stmt = con->createStatement();
	stmt->execute("DROP TABLE IF EXISTS inventory");
	cout << "Finished dropping table (if existed)" << endl;
	stmt->execute("CREATE TABLE inventory (id serial PRIMARY KEY, name VARCHAR(50), quantity INTEGER);");
	cout << "Finished creating table" << endl;
	delete stmt;

	pstmt = con->prepareStatement("INSERT INTO inventory(name, quantity) VALUES(?,?)");
	pstmt->setString(1, "banana");
	pstmt->setInt(2, 150);
	pstmt->execute();
	cout << "One row inserted." << endl;

	pstmt->setString(1, "orange");
	pstmt->setInt(2, 154);
	pstmt->execute();
	cout << "One row inserted." << endl;

	pstmt->setString(1, "apple");
	pstmt->setInt(2, 100);
	pstmt->execute();
	cout << "One row inserted." << endl;

	delete pstmt;
	delete con;
	system("pause");*/

	//sql::Driver* driver;
	//sql::Connection* con;
	//sql::PreparedStatement* pstmt;
	//sql::ResultSet* result;

	//try
	//{
	//	driver = get_driver_instance();
	//	//for demonstration only. never save password in the code!
	//	con = driver->connect(server, username, password);
	//}
	//catch (sql::SQLException e)
	//{
	//	cout << "Could not connect to server. Error message: " << e.what() << endl;
	//	system("pause");
	//	exit(1);
	//}

	//con->setSchema("quickstartdb");

	////select  
	//pstmt = con->prepareStatement("SELECT * FROM inventory;");
	//result = pstmt->executeQuery();

	//while (result->next())
	//	printf("Reading from table=(%d, %s, %d)\n", result->getInt(1), result->getString(2).c_str(), result->getInt(3));

	//delete result;
	//delete pstmt;
	//delete con;
	//system("pause");
	//return 0;
	return 0;
}