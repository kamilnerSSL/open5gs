# SMF YAML Configuration Syntax Specification

**Source of truth**: [`configs/open5gs/smf.yaml.in`](../configs/open5gs/smf.yaml.in),
[`src/smf/context.c`](../src/smf/context.c),
[`lib/pfcp/context.c`](../lib/pfcp/context.c)

---

## Top-level Structure

```yaml
logger:       # Logging configuration
global:       # Global pool/parameter settings (shared across all NFs)
smf:          # SMF-specific configuration
```

---

## `logger`

```yaml
logger:
  file:
    path: /var/log/open5gs/smf.log   # Path to log file
  level: info                         # fatal|error|warn|info(default)|debug|trace
```

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `file.path` | string | — | Log file path |
| `level` | enum | `info` | Log verbosity: `fatal`, `error`, `warn`, `info`, `debug`, `trace` |

---

## `global`

```yaml
global:
  max:
    ue: 1024    # Maximum number of UEs (increases memory usage)
    peer: 64    # Maximum number of peer connections
```

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `max.ue` | integer | 1024 | Max simultaneous UEs |
| `max.peer` | integer | 64 | Max peer connections |

---

## `smf`

All SMF-specific configuration lives under the `smf:` key.

```yaml
smf:
  sbi: ...
  pfcp: ...
  gtpc: ...
  gtpu: ...
  metrics: ...
  session: ...
  dns: ...
  mtu: ...
  p-cscf: ...
  ctf: ...
  freeDiameter: ...
  info: ...
  security_indication: ...
  default: ...
  nrf: ...
  scp: ...
  service_name: ...
  discovery: ...
```

---

### `smf.sbi` — Service Based Interface

Used for 5GC NF communication (Nsmf interface).

```yaml
smf:
  sbi:
    server:
      - address: 127.0.0.4          # IP address or hostname to bind
        port: 7777                  # TCP port (default: 7777)
        dev: eth0                   # OR bind to network interface
        advertise: open5gs-smf.svc.local        # Hostname/URI to advertise
        advertise: open5gs-smf.svc.local:8888   # Advertise with different port
    client:
      nrf:
        - uri: http://127.0.0.10:7777   # NRF URI for direct communication
      scp:
        - uri: http://127.0.0.200:7777  # SCP URI for indirect communication
      delegated:                        # Override delegation defaults
        nrf:
          nfm: no    # yes|no — delegate NRF management functions to SCP
          disc: yes  # yes|no — delegate NRF discovery to SCP
        scp:
          next: yes  # yes|no — delegate next-hop communications to SCP
```

#### `sbi.server[]` fields

| Key | Type | Description |
|-----|------|-------------|
| `address` | string | IP address or FQDN to bind (mutually exclusive with `dev`) |
| `port` | integer | TCP port number (default: 7777) |
| `dev` | string | Network interface name to bind to |
| `advertise` | string | Address/URI advertised to NRF; may include port (`host:port`) |

#### `sbi.client.nrf[]` / `sbi.client.scp[]` fields

| Key | Type | Description |
|-----|------|-------------|
| `uri` | string | Full URI including scheme, host, and port |

#### `sbi.client.delegated` fields

Controls which functions are delegated to NRF vs SCP.
If `delegated` is omitted, all communications are AUTO-delegated to SCP.

| Key | Type | Values | Description |
|-----|------|--------|-------------|
| `nrf.nfm` | bool | `yes`/`no` | Delegate NRF management to SCP |
| `nrf.disc` | bool | `yes`/`no` | Delegate NRF discovery to SCP |
| `scp.next` | bool | `yes`/`no` | Delegate next-hop routing to SCP |

---

### `smf.default` — TLS Defaults

Sets default TLS configuration applied to all SBI connections unless overridden.

```yaml
smf:
  default:
    tls:
      server:
        scheme: https
        private_key: /etc/open5gs/tls/smf.key
        cert: /etc/open5gs/tls/smf.crt
        verify_client: true
        verify_client_cacert: /etc/open5gs/tls/ca.crt
        sslkeylogfile: /var/log/open5gs/tls/smf-server-sslkeylog.log
      client:
        scheme: https
        cacert: /etc/open5gs/tls/ca.crt
        client_private_key: /etc/open5gs/tls/smf.key
        client_cert: /etc/open5gs/tls/smf.crt
        client_sslkeylogfile: /var/log/open5gs/tls/smf-client-sslkeylog.log
```

| Key | Type | Description |
|-----|------|-------------|
| `tls.server.scheme` | string | `http` or `https` |
| `tls.server.private_key` | string | Path to server private key (PEM) |
| `tls.server.cert` | string | Path to server certificate (PEM) |
| `tls.server.verify_client` | bool | Require client certificate verification |
| `tls.server.verify_client_cacert` | string | CA cert for verifying client certificates |
| `tls.server.sslkeylogfile` | string | Path to write TLS session keys (Wireshark use) |
| `tls.client.scheme` | string | `http` or `https` |
| `tls.client.cacert` | string | CA certificate for verifying server |
| `tls.client.client_private_key` | string | Path to client private key (mTLS) |
| `tls.client.client_cert` | string | Path to client certificate (mTLS) |
| `tls.client.client_sslkeylogfile` | string | Path to write client TLS session keys |

---

### `smf.pfcp` — PFCP (Packet Forwarding Control Protocol)

Controls N4 interface between SMF and UPF.

```yaml
smf:
  pfcp:
    server:
      - address: 127.0.0.4          # IP to bind PFCP server
        port: 8805                  # UDP port (default: 8805)
        dev: eth0                   # OR bind to interface
        advertise: open5gs-smf.svc.local  # Address advertised to UPF
    client:
      upf:
        - address: 127.0.0.7        # UPF PFCP address
          port: 8805                # Optional: override default port
          tac: 1                    # UPF selection: single TAC (decimal)
          tac: [3, 5, 8]            # UPF selection: multiple TACs
          dnn: ims                  # UPF selection: single DNN/APN
          dnn: [internet, web]      # UPF selection: multiple DNNs
          e_cell_id: 463            # UPF selection: eNodeB Cell ID (28-bit hex)
          e_cell_id: [463, 464]
          nr_cell_id: 123456789     # UPF selection: NR Cell ID (36-bit hex)
          nr_cell_id: [123456789, 9413]
```

#### `pfcp.server[]` fields

| Key | Type | Description |
|-----|------|-------------|
| `address` | string | IP address or FQDN to bind |
| `port` | integer | UDP port (default: 8805) |
| `dev` | string | Network interface to bind to |
| `advertise` | string | Address advertised in PFCP Association |
| `family` | integer | Address family: `2` (AF_INET) or `10` (AF_INET6) |

#### `pfcp.client.upf[]` fields

| Key | Type | Description |
|-----|------|-------------|
| `address` | string | UPF PFCP endpoint address or FQDN |
| `port` | integer | UDP port (default: 8805) |
| `family` | integer | Address family override |
| `tac` | integer or list | TAC(s) this UPF handles (decimal); used for UPF selection |
| `dnn` / `apn` | string or list | DNN/APN(s) this UPF handles; used for UPF selection |
| `e_cell_id` | hex or list | eNodeB Cell ID(s) (28-bit, hex); used for UPF selection |
| `nr_cell_id` | hex or list | NR Cell ID(s) (36-bit, hex); used for UPF selection |

**UPF selection priority**: If a UPF has `tac`, `dnn`, `e_cell_id`, or `nr_cell_id` set, it is
selected for sessions matching those criteria. If multiple UPFs match (or none have selection
criteria), round-robin is used.

---

### `smf.gtpc` — GTP-C Server (EPC / 4G)

Listens for GTPv1/GTPv2-C messages from MME/SGW.

```yaml
smf:
  gtpc:
    server:
      - address: 127.0.0.4
      - address: fd69:f21d:873c:fa::3   # IPv6
```

#### `gtpc.server[]` fields

| Key | Type | Description |
|-----|------|-------------|
| `address` | string | IPv4 or IPv6 address to listen on |
| `port` | integer | UDP port (default: 2123) |
| `dev` | string | Network interface to bind to |

---

### `smf.gtpu` — GTP-U Server

Listens for GTP-U traffic (used in some SMF deployments).

```yaml
smf:
  gtpu:
    server:
      - address: 127.0.0.4
      - address: ::1
```

#### `gtpu.server[]` fields

| Key | Type | Description |
|-----|------|-------------|
| `address` | string | IPv4 or IPv6 address |
| `port` | integer | UDP port (default: 2152) |
| `dev` | string | Network interface to bind to |

---

### `smf.metrics` — Prometheus Metrics

```yaml
smf:
  metrics:
    server:
      - address: 127.0.0.4
        port: 9090
```

| Key | Type | Description |
|-----|------|-------------|
| `server[].address` | string | Address to expose metrics HTTP endpoint |
| `server[].port` | integer | Port for Prometheus scraping (default: 9090) |

---

### `smf.session` — UE IP Pool / Session Configuration

Defines IP address pools allocated to UEs and links them to DNNs.

```yaml
smf:
  session:
    - subnet: 10.45.0.0/16           # Required: IP subnet for UE pool
      gateway: 10.45.0.1             # Optional: gateway IP (defaults to first addr)
      dnn: internet                  # Optional: bind pool to this DNN/APN
      dev: ogstun                    # Optional: TUN interface name
      range:                         # Optional: restrict allocation to sub-ranges
        - 10.45.0.100-10.45.0.200   # start-end (inclusive)
        - 10.45.1.100-               # start to end of subnet
        - -10.45.0.200               # start of subnet to end

    - subnet: 2001:db8:cafe::/48     # IPv6 subnet
      gateway: 2001:db8:cafe::1
      dnn: internet
      range:
        - 2001:db8:cafe:a0::0-2001:db8:cafe:b0::0
```

#### `session[]` fields

| Key | Type | Required | Description |
|-----|------|----------|-------------|
| `subnet` | CIDR string | Yes | IP subnet in `address/prefix` format; supports both IPv4 and IPv6 |
| `gateway` | IP string | No | Gateway address; defaults to first usable address in subnet |
| `dnn` / `apn` | string | No | DNN or APN this pool is bound to; if omitted, pool is used for any DNN |
| `dev` | string | No | TUN interface name (default: `ogstun`) |
| `range` | list of strings | No | Restrict allocations to sub-ranges; format: `start-end`, `start-` (open end), or `-end` (open start) |

**Note**: If a UE requests a DNN with no matching pool, the SMF/UPF will error. At minimum, one
pool without a `dnn` filter (or with `dnn: internet`) is required.

---

### `smf.dns` — DNS Servers Advertised to UEs

Up to 2 IPv4 and 2 IPv6 addresses. Additional entries are ignored with a warning.

```yaml
smf:
  dns:
    - 8.8.8.8
    - 8.8.4.4
    - 2001:4860:4860::8888
    - 2001:4860:4860::8844
```

| Type | Max entries | Description |
|------|-------------|-------------|
| IPv4 | 2 | Primary and secondary IPv4 DNS |
| IPv6 | 2 | Primary and secondary IPv6 DNS |

---

### `smf.mtu` — MTU Advertised to UEs

```yaml
smf:
  mtu: 1400
```

| Key | Type | Description |
|-----|------|-------------|
| `mtu` | integer | MTU value sent to UE in PCO (Protocol Configuration Options) |

---

### `smf.p-cscf` — P-CSCF Addresses (IMS)

Proxy Call Session Control Function addresses advertised to UEs for IMS. Accepts IPv4, IPv6,
or FQDNs (resolved at startup). Max `MAX_NUM_OF_P_CSCF` entries per address family.

```yaml
smf:
  p-cscf:
    - 127.0.0.1
    - ::1
    - pcscf.ims.mnc070.mcc999.3gppnetwork.org
```

---

### `smf.ctf` — Charging Trigger Function

Controls whether charging interfaces are activated.

```yaml
smf:
  ctf:
    enabled: auto    # auto(default)|yes|no
```

| Value | Description |
|-------|-------------|
| `auto` | Enable charging if `freeDiameter` is configured (default) |
| `yes` | Always enable charging |
| `no` | Always disable charging |

---

### `smf.freeDiameter` — Diameter Protocol (EPC / Gy/Ro charging)

Can be a simple path string or an expanded mapping.

**Simple form** (path to freeDiameter config file):
```yaml
smf:
  freeDiameter: /etc/freeDiameter/smf.conf
```

**Expanded form** (inline configuration):
```yaml
smf:
  freeDiameter:
    identity: smf.localdomain
    realm: localdomain
    port: 3868
    sec_port: 5868
    listen_on: 127.0.0.4
    no_fwd: true
    tc_timer: 30
    load_extension:
      - module: /usr/lib/freeDiameter/dict_rfc5777.fdx
        conf: /etc/freeDiameter/rfc5777.conf   # optional
    connect:
      - identity: ocs.localdomain
        address: 127.0.0.9
        port: 3868
        tc_timer: 30
```

#### `freeDiameter` mapping fields

| Key | Type | Description |
|-----|------|-------------|
| `identity` | string | Diameter identity (FQDN) of this node |
| `realm` | string | Diameter realm |
| `port` | integer | Diameter plaintext port (default: 3868) |
| `sec_port` | integer | Diameter TLS port (default: 5868) |
| `listen_on` | string | IP address to bind Diameter listener |
| `no_fwd` | bool | Disable message forwarding (relay mode) |
| `tc_timer` | integer | Tc timer value in seconds (reconnect interval) |
| `load_extension[].module` | string | Path to freeDiameter extension `.fdx` file |
| `load_extension[].conf` | string | Optional config file for that extension |
| `connect[].identity` | string | Diameter identity of peer to connect to |
| `connect[].address` | string | IP address of peer |
| `connect[].port` | integer | Port of peer |
| `connect[].tc_timer` | integer | Per-peer Tc timer override |

---

### `smf.info` — SMF Info for NF Selection (5GC only)

Advertised to NRF so AMF can select this SMF for specific S-NSSAIs, DNNs, and TAIs.
If omitted, any AMF can select this SMF.

```yaml
smf:
  info:
    - s_nssai:
        - sst: 1                    # Slice/Service Type (integer, 1–255)
          sd: 009000                # Slice Differentiator (6 hex digits, optional)
          dnn:
            - internet              # At least 1 DNN required per S-NSSAI
            - ims
        - sst: 2
          dnn:
            - internet
      tai:                          # Optional: restrict to tracking areas
        - plmn_id:
            mcc: 999
            mnc: 70
          tac: 1                    # Single TAC
        - plmn_id:
            mcc: 999
            mnc: 70
          tac: [1, 2, 3]            # Multiple specific TACs
        - plmn_id:
            mcc: 999
            mnc: 70
          tac:
            - 10-20                 # TAC range (inclusive)
            - 30-40
    - s_nssai:                      # Additional info entry for different conditions
        - sst: 4
          dnn:
            - internet
      tai:
        - plmn_id:
            mcc: 999
            mnc: 70
          tac: 99
```

#### `info[]` fields

| Key | Type | Description |
|-----|------|-------------|
| `s_nssai[]` | list | One or more S-NSSAI entries this SMF serves |
| `s_nssai[].sst` | integer | Slice/Service Type (1–255) |
| `s_nssai[].sd` | hex string | Slice Differentiator, 6 hex digits, e.g. `009000` (optional) |
| `s_nssai[].dnn` | list of strings | DNNs served by this slice; at least one required |
| `tai[]` | list | Optional TAI constraints; if absent, SMF serves all TAIs |
| `tai[].plmn_id.mcc` | string | Mobile Country Code |
| `tai[].plmn_id.mnc` | string | Mobile Network Code |
| `tai[].tac` | integer, list, or range string | TAC value(s); range syntax: `"low-high"` |

**TAC range syntax**: A string `"10-20"` sets a range from 10 to 20 inclusive. `"5"` is a single
value. Leading `-` means "from subnet start"; trailing `-` means "to subnet end".

---

### `smf.security_indication` — UP Security (5GC only)

Configures UP (User Plane) security requirements signalled to the RAN via NAS/NGAP.

```yaml
smf:
  security_indication:
    integrity_protection_indication: required    # required|preferred|not-needed
    confidentiality_protection_indication: preferred
    maximum_integrity_protected_data_rate_uplink: maximum-UE-rate
    maximum_integrity_protected_data_rate_downlink: bitrate64kbs
```

| Key | Type | Values | Description |
|-----|------|--------|-------------|
| `integrity_protection_indication` | enum | `required`, `preferred`, `not-needed` | UP integrity protection requirement |
| `confidentiality_protection_indication` | enum | `required`, `preferred`, `not-needed` | UP confidentiality protection requirement |
| `maximum_integrity_protected_data_rate_uplink` | enum | `bitrate64kbs`, `maximum-UE-rate` | Max uplink data rate for integrity-protected bearers |
| `maximum_integrity_protected_data_rate_downlink` | enum | `bitrate64kbs`, `maximum-UE-rate` | Max downlink data rate for integrity-protected bearers |

---

### `smf.nrf` / `smf.scp` — Shorthand NRF/SCP Config

These keys are handled by the SBI library and are an alternative way to specify NRF/SCP
endpoints (equivalent to `sbi.client.nrf` / `sbi.client.scp`).

```yaml
smf:
  nrf:
    - uri: http://127.0.0.10:7777
  scp:
    - uri: http://127.0.0.200:7777
```

---

### `smf.service_name` — Advertised SBI Service Names

Controls which NF service names are registered with the NRF.

```yaml
smf:
  service_name:
    - nsmf-pdusession
    - nsmf-event-exposure
```

---

### `smf.discovery` — NF Discovery Configuration

```yaml
smf:
  discovery:
    option:
      no_delegation: true   # Do not delegate discovery to SCP
```

---

## Complete Minimal Example

```yaml
logger:
  file:
    path: /var/log/open5gs/smf.log

global:
  max:
    ue: 1024

smf:
  sbi:
    server:
      - address: 127.0.0.4
        port: 7777
    client:
      scp:
        - uri: http://127.0.0.200:7777
  pfcp:
    server:
      - address: 127.0.0.4
    client:
      upf:
        - address: 127.0.0.7
  gtpc:
    server:
      - address: 127.0.0.4
  gtpu:
    server:
      - address: 127.0.0.4
  metrics:
    server:
      - address: 127.0.0.4
        port: 9090
  session:
    - subnet: 10.45.0.0/16
      gateway: 10.45.0.1
    - subnet: 2001:db8:cafe::/48
      gateway: 2001:db8:cafe::1
  dns:
    - 8.8.8.8
    - 8.8.4.4
    - 2001:4860:4860::8888
    - 2001:4860:4860::8844
  mtu: 1400
  freeDiameter: /etc/freeDiameter/smf.conf
```
