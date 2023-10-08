// Run status
enum class RunStatus : char
{
    Idle = '1',
    Running,
    Recovering
};

extern RunStatus run_status;

