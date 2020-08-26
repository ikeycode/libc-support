getent_sources = [
    'getent.c',
    'db_gshadow.c',
    'db_initgroups.c',
    'db_shadow.c',
    'db_rpc.c',
    'db_networks.c',
    'db_ethers.c',
    'db_password.c',
    'db_hosts.c',
    'db_protocols.c',
    'db_services.c',
    'db_aliases.c',
    'db_netgroup.c',
    'db_group.c',
]

executable('getent',
    sources: getent_sources,
    install: true,
    include_directories: root_includedir,
)
