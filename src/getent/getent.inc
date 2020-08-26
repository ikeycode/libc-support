        DATABASE_CONF_HOSTS(ahosts),
        DATABASE_CONF_HOSTS(ahostsv4),
        DATABASE_CONF_HOSTS(ahostsv6),
        DATABASE_CONF_HOSTS(hosts),
        DATABASE_CONF(networks),
        DATABASE_CONF(password),
        DATABASE_CONF(protocols),
        DATABASE_CONF(ethers),
        DATABASE_CONF(group),
        DATABASE_CONF(services),
        DATABASE_CONF(shadow),
#if HAVE_ALIASES
        DATABASE_CONF(aliases),
#endif
#if HAVE_GSHADOW
        DATABASE_CONF(gshadow),
#endif
        DATABASE_CONF(initgroups),
#if HAVE_NETGROUP
        DATABASE_CONF(netgroup),
#endif
#if HAVE_RPC
        DATABASE_CONF(rpc),
#endif
