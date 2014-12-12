/**
 *  @file raii_connection_test.cpp
 *  @date 21.03.2014
 *  @author mkoenig
 *  @brief 
 *
 *  $LastChangedDate: 2014-11-28 15:26:51 +0100 (Fr, 28 Nov 2014) $
 *  $LastChangedBy: mkoenig $
 *  $LastChangedRevision: 21210 $
 *
 */


#include "cpp_odbc/raii_connection.h"

#include <cppunit/extensions/HelperMacros.h>

#include "cpp_odbc/raii_statement.h"
#include "cpp_odbc_test/level2_mock_api.h"
#include "cpp_odbc_test/level2_dummy_api.h"

#include <type_traits>

class raii_connection_test : public CppUnit::TestFixture {
CPPUNIT_TEST_SUITE( raii_connection_test );

	CPPUNIT_TEST( is_connection );
	CPPUNIT_TEST( raii_connect_and_disconnect );
	CPPUNIT_TEST( get_api );
	CPPUNIT_TEST( get_handle );

	CPPUNIT_TEST( make_statement );
	CPPUNIT_TEST( set_connection_attribute );
	CPPUNIT_TEST( commit );
	CPPUNIT_TEST( rollback );
	CPPUNIT_TEST( get_string_info );

CPPUNIT_TEST_SUITE_END();

public:

	void is_connection();
	void raii_connect_and_disconnect();
	void get_api();
	void get_handle();

	void make_statement();
	void set_connection_attribute();
	void commit();
	void rollback();
	void get_string_info();
};

// Registers the fixture with the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( raii_connection_test );

using cpp_odbc::raii_connection;
using cpp_odbc::raii_statement;
using cpp_odbc_test::level2_mock_api;
using cpp_odbc::level2::environment_handle;
using cpp_odbc::level2::connection_handle;
using cpp_odbc::level2::statement_handle;

namespace {

	// destinations for pointers, values irrelevant
	int value_a = 17;
	int value_b = 23;
	int value_c = 42;

	environment_handle const e_handle = {&value_a};
	connection_handle const default_c_handle = {&value_b};
	statement_handle const default_s_handle = {&value_c};


	psapp::valid_ptr<testing::NiceMock<level2_mock_api>> make_default_api()
	{
		auto api = psapp::make_valid_ptr<testing::NiceMock<level2_mock_api>>();

		ON_CALL(*api, do_allocate_connection_handle(testing::_))
			.WillByDefault(testing::Return(default_c_handle));

		return api;
	}

}

void raii_connection_test::is_connection()
{
	bool const derived_from_connection = std::is_base_of<cpp_odbc::connection, raii_connection>::value;
	CPPUNIT_ASSERT( derived_from_connection );
}


void raii_connection_test::raii_connect_and_disconnect()
{
	auto api = psapp::make_valid_ptr<cpp_odbc_test::level2_mock_api>();
	connection_handle c_handle = {&value_a};

	std::string const connection_string = "my DSN";

	EXPECT_CALL(*api, do_allocate_connection_handle(e_handle)).
			WillOnce(testing::Return(c_handle));
	EXPECT_CALL(*api, do_establish_connection(c_handle, connection_string)).Times(1);

	{ // scope introduced for RAII check
		raii_connection connection(api, e_handle, connection_string);
		EXPECT_CALL(*api, do_disconnect(c_handle)).Times(1);
		EXPECT_CALL(*api, do_free_handle(c_handle)).Times(1);
	}
}

void raii_connection_test::get_api()
{
	auto expected_api = psapp::make_valid_ptr<cpp_odbc_test::level2_dummy_api const>();

	raii_connection instance(expected_api, e_handle, "dummy");
	CPPUNIT_ASSERT( expected_api == instance.get_api());
}

void raii_connection_test::get_handle()
{
	auto api = psapp::make_valid_ptr<cpp_odbc_test::level2_dummy_api>();

	std::string const connection_string = "my DSN";
	raii_connection instance(api, e_handle, connection_string);

	bool const returns_handle_ref = std::is_same<connection_handle const &, decltype(instance.get_handle())>::value;

	CPPUNIT_ASSERT( returns_handle_ref );
	instance.get_handle(); // make sure function symbol is there
}

void raii_connection_test::make_statement()
{
	auto api = make_default_api();
	auto connection = std::make_shared<raii_connection>(api, e_handle, "dummy");
	EXPECT_CALL(*api, do_allocate_statement_handle(default_c_handle))
		.WillOnce(testing::Return(default_s_handle));

	auto statement = connection->make_statement();
	bool const is_raii_statement = (std::dynamic_pointer_cast<raii_statement>(statement) != nullptr);
	CPPUNIT_ASSERT( is_raii_statement );
}

void raii_connection_test::set_connection_attribute()
{
	SQLINTEGER const attribute = 42;
	long const value = 1234;

	auto api = make_default_api();
	raii_connection connection(api, e_handle, "dummy");
	EXPECT_CALL(*api, do_set_connection_attribute(default_c_handle, attribute, value))
		.Times(1);

	connection.set_connection_attribute(attribute, value);
}

void raii_connection_test::commit()
{
	auto api = make_default_api();
	raii_connection connection(api, e_handle, "dummy");
	EXPECT_CALL(*api, do_end_transaction(default_c_handle, SQL_COMMIT))
		.Times(1);

	connection.commit();
}

void raii_connection_test::rollback()
{
	auto api = make_default_api();
	raii_connection connection(api, e_handle, "dummy");
	EXPECT_CALL(*api, do_end_transaction(default_c_handle, SQL_ROLLBACK))
		.Times(1);

	connection.rollback();
}

void raii_connection_test::get_string_info()
{
	SQLUSMALLINT const info_type = 42;
	std::string const expected("dummy");

	auto api = make_default_api();
	raii_connection connection(api, e_handle, "dummy");
	EXPECT_CALL(*api, do_get_string_connection_info(default_c_handle, info_type))
		.WillOnce(testing::Return(expected));

	CPPUNIT_ASSERT_EQUAL(expected, connection.get_string_info(info_type));
}
