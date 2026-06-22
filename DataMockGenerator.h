#ifndef DATAMOCKGENERATOR_H
#define DATAMOCKGENERATOR_H

#include "Customer.h"
#include "ICustomerRepository.h"

#include <QStringList>
#include <vector>

class DataMockGenerator
{
public:
    static std::vector<Customer> generateCustomers(int count, const QStringList& ownerIds = {});
    static bool appendToRepository(ICustomerRepository& repository, int count, const QStringList& ownerIds = {});
};

#endif // DATAMOCKGENERATOR_H
