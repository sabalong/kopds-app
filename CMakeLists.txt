cmake_minimum_required(VERSION 3.25)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project(kopds_app VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Wt CONFIG REQUIRED)

add_executable(kopds-app
  src/main.cpp
  src/bootstrap/KopdsApplication.cpp
  src/shared/infrastructure/Database.cpp
  src/shared/presentation/ConfirmationDialog.cpp
  src/contexts/membership/application/MemberService.cpp
  src/contexts/membership/infrastructure/PostgresMemberRepository.cpp
  src/contexts/membership/presentation/MembersPage.cpp
  src/contexts/contribution_configuration/application/ContributionTypeService.cpp
  src/contexts/contribution_configuration/infrastructure/PostgresContributionTypeRepository.cpp
  src/contexts/contribution_configuration/presentation/ContributionTypesPage.cpp
  src/contexts/system_health/application/GetSystemStatus.cpp
  src/contexts/system_health/infrastructure/PostgresHealthCheck.cpp
  src/contexts/system_health/presentation/SystemStatusPage.cpp
  src/shell/AppShell.cpp
  src/shell/DashboardPage.cpp
  src/shell/NotFoundPage.cpp
)

target_include_directories(kopds-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(kopds-app
  PRIVATE
    Wt::Wt
    Wt::HTTP
    Wt::Dbo
    Wt::DboPostgres
)

target_compile_features(kopds-app PRIVATE cxx_std_17)
