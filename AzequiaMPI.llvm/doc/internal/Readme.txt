---------------------------------------------------------------------
 IDSP by Juan Carlos Díaz Martín
---------------------------------------------------------------------

VERSIÓN idsp_811

. Construye un manejador de dispositivo DSP/BIOS sobre el puerto 
  serie en modo loopback utilizando el EDMA: /lnk

. Construye IDSP, un "framework" distribuido para aplicaciones DSP. 

. Lleva el control de la utilización instantánea del procesador consistente con el "CPU LOAD graph" de DSP/BIOS. De esta forma, cada nueva instancia de operador se crea en la máquina menos cargada. Este servicio se proporciona en el módulo "isdp_cpu_load.c".

. Construye:
  Una interacción send/receive de cita extendida (en el sentido Ada95) para tareas que residen en la misma UCP. 
  Una interacción send/receive con buffer para tareas que residen en distintos procesadores. Send se comporta de forma asíncrona, enviando el mensaje al procesador remoto. Receive es síncrono tanto en modo local como remoto. 
  Una sintaxis homogénea de comunicación, con independencia de la ubicación de las tareas, sea local o remota.

. Introduce una interfaz de plazos en envío y recepción. El usuario determina el plazo cuando crea un comunicador
 
. Termina las sesiones de una aplicación liberando los recursos consumidos por esta -sólo aquellos que gestiona el núcleo-. Así, se liberan descriptores de operadores y sesión.

. Cuando termina una sesión, la biblioteca "IDSP_channel" libera los recursos asociados a los canales -gestinados estos como biblioteca de usuario-, así como los recursos relativos a los comunicadores. 

. Un mecanismo de excepciones.

. Elimina la necesidad de invocar "IDSP_init" en los hilos de usuario. 

. Funde los ficheros fuente "idsp_k_operator.c" y "idsp_k_communicator.c" en un fichero fuente único, más grande, "idsp_kernel.c".

. Elimina la dependencia del driver de bajo nivel "mcbsp_edma.c" respecto a "idsp_kernel.h". Ahora el driver es mucho más independiente.

. Mejora el tratamiento de excepciones mostrando el identificador de la tarea que produjo la excepción en el mensaje de excepción.

. Introduce un ejemplo de comunicación de vectores en "user_operators.c". Hasta ahora sólo se había utilizado la comunicación de valores enteros.

. Mejora el módulo "mcbsp_edma.c". Este exporta ahora la constante MCBSP_EDMA_MAX_FRAME_SIZE, el marco más grande que es capaz de enviar.

. IDSP_Channel_Send es ahora capaz de enviar un número cualquiera de octetos. Antes sólo podía enviar un número de octetos igual o inferior al calibre de canal. Además, devuelve el número de octetos enviados. 

. La dependencia entre niveles es ahora enteramente correcta. Un nivel sólo depende ahora del inmediatamnete inferior. El nivel DLN ya no depende del nivel IDSP. 

. En consecuencia, la función Idsp_Map_addr_to_stream ha desaparecido. Ahora no es invocada por DLN para distribuir los marcos entrantes al operador objetivo. La solución ha consistido en la tabla "addr2spec", basada en el concepto de "Dirección lineal". No hay huecos en el espacio de direcciones de los comunicadores. EStas forman un continuo entre 0 y un máximo. Básicamente, cuando llega un marco al nivel DLN, este calcula su dirección lineal a partir del campo "dirección destino" del marco. Con ella indexa la tabla "addr2spec" para recuperar el objeto específico del SIO de entrada del comunicador objetivo. A partit del objeto específico se extae el semáforo donde espera el operador destino, subiéndolo. De esta forma el operador destinatario del marco entrante despierta. 

. Opera con la versión v2 de Code Composer.

. El mecanismo de excepciones se ha culminado. Su uso es como sigue:

function() 
{
  ... /* La excepción se detecta... */
  raise(LOCAL_E_0);  /* ... y se eleva */
  ...
  return(0);

IDSP_EXCEPTION_BEGIN
    exception(LOCAL_E_0):
      // your exception code
      reraise(CHANNEL_E_BAD_PARM);
    ...  
    exception(...):
      ...
IDSP_EXCEPTION_END
}

. La creación y destrucción de comunicadores se hace de manera estructurada, orientada a la correcta asignación y liberación de recursos de recursos y combinada con el mecanismo de excepciones. 

. El fuente "Mcbsp_Edma.c" se ha mejorado. Ahora opera con la velocidad del mcbsp más baja posible, la dada por el divisor de reloj 255. Se ha conseguido desplazando la interrupción del EDMA al buffer que inserta la cola en el marco saliente (ver la función Start_Send_EDMA).

. Se ha acuñado el concepto de célula como el conjunto de operadores de una sesión asignados a la misma máquina. La antigua gestión de las "sesiones" que antes hacía el kernel se denomina ahora gestión de células. La gestión de células se ha sacado del kernel que ahora sólo sabe de operadores y comunicadores.

. La gestión de células la lleva ahora enteramente el servidor. Se ha diseñado un interface de gestión de células ("idsp_cell.h") de tres primitivas:

  Int CELL_create (Idsp_Cell_t *cell, Idsp_Cell_Addr_t cellAddr);
  Int CELL_destroy(Idsp_Cell_t  cell, Idsp_Cell_Addr_t cellAddr);
  Int CELL_add    (Idsp_Cell_t  cell, Int              operCode, 
                                      Idsp_Oper_Addr_t operAddr); 

. Se ha introucido en IDSP el mecanismo de llamada a procedimiento remoto (RPC). Idealmente dispondriamos de un fichero de "definición de interface" de los métodos arriba citados CELL_xx. La compilación de este fichero hipotético resultaría en dos cabos (stubs): Los ficheros fuente:

Cabo cliente:  "idsp_cell_clnt.c"  
Cabo servidor: "idsp_server.c" 
 
Ambos ficheros se han escrito "a mano". El servidor IDSP actúa como el cabo servidor. La implementación de los métodos se lleva a cabo en el fichero "idsp_cell_svr.c". Para evitar la colisión de nombres con "idsp_cell_clnt.c", no ha habido más remedio que renombrar las funciones CELL_xx como CELL_SVR_xx. Así, cuando el shell invoca el método CELL_create del cabo cliente, este es servido por el método CELL_SVR_create en la máquina remota.

Utiliza el cabo cliente de momento el shell. El uso de RPC en IDSP proporciona una arquitectura más definida y robusta:

1> El shell se ha simplificado sustancialmente utilizando llamadas RPC. La razón es que ahora no construye y envía mensajes, sino que se limita a invocar el interface CELL_xx.

2> El servidor también se ha simplificado. Ahora se limita a recojer mensajes y desmontarlos para invocar los métodos del interface CELL_SVR_xx, métodos implementados en "idsp_cell_svr.c".

3> El kernel se ha simplifacado por que la gestión de células se lleva a cabo  ahora en "idsp_cell_svr.c". Su interface tiene ahora 9 primitivas. Cuatro de gestión de operadores:

  Idsp_Oper_t 
     IDSP_OPER_self      (void                        ); 
  Int IDSP_OPER_create   (Idsp_Oper_t      *oper,  
                          Int               operCode,
                          Idsp_Oper_Addr_t  addr      );
  Int IDSP_OPER_destroy  (Idsp_Oper_t       oper      );
  Int IDSP_OPER_disable  (Idsp_Oper_t       oper      );  

y cinco de gestión de comunicación:                 

  Int IDSP_COMM_create   (Int              *comm, 
                          Uns               port, 
                    const Idsp_Comm_Attr_t  attr, 
                          Idsp_Oper_t       inst      ); 
  Int IDSP_COMM_destroy  (Int  comm, Idsp_Oper_t inst ); 
  Int IDSP_COMM_receive  (Int  comm, Idsp_Comm_Addr_t *src,  Ptr *buffer, 
                                                             Int *nbytes); 
  Int IDSP_Comm_send     (Int  comm, Idsp_Comm_Addr_t  dst,  Ptr  buffer, 
                                                             Int  nbytes);  
  Int IDSP_COMM_sendrec  (Int  comm, Idsp_Comm_Addr_t *addr, Ptr *buffer, 
                                                             Int *nbytes); 

. La actuación sobre el edma para el envío se ha simplificado con un canal mudo. Ha desaparecido la interrupción de deshabilitación del EDMA.

. MCBSP_edma.c pasa a denominarse idsp_link.c. Dln.c pasa a denominarse idsp_net.c. 

. Se ha ocultado el concepto de célula al usuario. Ya no existe un servidor de células. La gestión de estas se lleva a cabo en el núcleo, como apoyo a la creación y destrucción de operadores. Para ello se utiliza el paquete idsp_cell.c.

. Se ha introducido un mecanismo de transparencia a la ubicación de los operadores. La dirección consiste en la terna [gix, oix, cix]: 
  . Gix es un identificador de grupo y es único a nivel de sistema. Es asignado por el servidor 
    de grupos cuando crea el grupo. Se asigna de forma creciente: 0, 1, 2, 3, ...
  . Oix es el número de orden del operador en el grupo: 0, 1, 2, 3, ..., O_MAX
  . Cix es el número de puerto del operador. Varía entre 0 y C_MAX

Observese que no existe un componente de máquina en la dirección.

. Ahora un thread DSP/BIOS es capaz de crear y destruir grupos. Para ello se ha creado el servicio de grupos, con la interfaz

  Int GROUP_create (Group_t *group, Int appCode);
  Int GROUP_destroy(Group_t  group);

GROUP_create es un stub cliente y como tal necesita enviar un mensaje. Para que una tarea DSP/BIOS pueda enviar mensajes, es preciso que disponga de comunicadores y, para ello, debe ser un operador IDSP. Para convertir una tarea DSP/BIOS en un operador IDSP y viceversa, se ha creado la interfaz 

  OPER_upgrade();
  OPER_degrade();

Tras la invocación de OPER_upgrade, una tarea DSP/BIOS puede invocar GROUP_create.

. La gestión de grupos se ha mejorado. La asignación del número de grupo se hace de forma circular.

. Se ha creado un servicio RPC para invocar la biblioteca C de entrada/salida. De momento, funciona únicamente una forma restringida de printf. Mediante este servicio es posible escribir una cadena de caracteres en la máquina donde resida el servidor RPC. El servicio se usa de forma:

  CIO_printf("Una cadena);

. Idsp_25 ejecuta con la memoria caché en modo 3-way caché. El buffer EDMA de entrada de datos "Cyclic_Buffer" se ha dispuesto en la memoria interna que queda, es decir, los primeros 16Kb. De esta forma no hay problemas de sincronización (Caché-Edma) que eran antes imposibles de solventar. Esta es una buena manera. El "visual linker" ha sido una herramienta muy útil en esta tarea.


. El EDMA del TMS320C6711 es un recurso utilizado tanto por los operadores de audio (nivel de usuario) como por el nivel de enlace (nivel de sistema), de modo que presenta conflictos potenciales. Sabemos que Si se desea que un canal EDMA invoque una interrupción específica cuando se ha completado la transferencia, debe pasarse un código TCC (Transfer Complete Code) a la rutina CSL que configura dicho canal EDMA (EDMA_ConfigB es una de estas rutinas). Cuando el EDMA interrumpe, su rutina de interrupción (universal) invocará las rutinas de interrupción específicas a los TCC activos en registro CIPR. 

Hay 16 códigos TCC en un C6711 (entre 0 y 15) de modo que estos 16 códigos puede considerarse como 16 recursos. Para gestionarlos se ha creado un registro de los códigos que se van asignando. Cuando se desea un TCC se solicita al registro. La gestión del registro la hace el nuevo paquete "EDMA_ISR". 

Un ejemplo: Tanto el nivel de enlace como el operador de usuario utilizan sus servicios para registrar una rutina de interrupción del EDMA. El operador de audio necesita que el EDMA interrumpa cuando se ha producido una operación "pon" (se han recibido 64 muestras del micrófono) o "quita" (Se han sacado 64 muestras al altavoz). Por su parte, el nivel de enlace necesita una interrupción EDMA que le informe de que el último paquete saliente ha sido puesto definitivamente en el cable. Ambas rutinas de interrupción entran en conflicto. 

El paquete ha sido implementado en los ficheros fuente C "idsp_edma_isr.c" y "idsp_edma_isr.h". Su interfaz es:

  void EDMA_ISR_isr        (void                                  );
  void EDMA_ISR_init       (void                                  );
  Int  EDMA_ISR_register   (Int *tcc, Fxn function, Int p1, Int p2);
  Int  EDMA_ISR_unregister (Int  tcc                              );

EDMA_ISR_isr es la rutina de interrupción del EDMA que es instalada por el software de configuración (ver idsp.cdb). Invoca a las rutinas registradas en función de los bits activos en el registro CIPR.   

EDMA_ISR_register toma cuatro parámetros:
a) EDMA_ISR_register devuelve en el primer parámetro un código TCC disponible. Estos números se otorgan de modo consecutivo, desde "0" hasta "TCC_MAX - 1". 
b) El segundo es la rutina de interrupción que se registra y el tercero y el cuarto dos parámetros optativos de la misma. 



. El paquete "DSK_CODEC_STREAM" se ha mejorado. Ahora toma énfasis el denominado "descriptor de dispositivo" Limited_Buffer. El método DSK_CODEC_STREAM_init ha desaparecido porque ya no se necesita.

. Un operador puede destruir un grupo si dispone de su identificador de grupo. Para destruir su propio grupo, se ha implementado la primitiva

  Int GROUP_self   (Group_t *group);

. Se ha hecho un estudio teórico sobre el time-out de un canal. Ver gráfica del 27 de Julio de 2002.

. Se ha creado la aplicación AUDIO_SPLIT. Consta de dos operadores, microphone y speaker. El primero lee del codec y envía un paquete de 64 muestras al segundo, que las envía al codec. La aplicación funciona correctamente, de modo que no se observa retardo alguno en la reproducción de la voz. Las muestras recorren toda la pila de comunicaciones. Con ello queda demostrado que IDSP opera en tiempo real. El resultado teórico del estudio anterior se ha aplicado a AUDIO_SPLIT, que corrobora la teoría. El estudio ha permitido inferir que se necesitan 3 ítems en el buffer acotado del operador SPEAKER_OPERATOR. Efectivamnete, con tres ítems SPEAKER_OPERATOR no se bloquea en el buffer acotado. Con dos ítems, el funcionamiento es perfecto también a pesar del bloqueo de SPEAKER_OPERATOR en el buffer.  

. Se ha dispuesto un mecanismo de garantía de calidad de servicio (QOS) en el buffer Cyclic_Buffer, de forma que haya espacio suficiente en el mismo en el peor caso. Este se produce en el instante crítico en que TODOS los canales de entrada reciben datos. Esto significa una disminución sustancial del número de opeadores que puede acojer una máquina cuando se crean los grupos. Para ello se ha dispuesto para el mandato de control

   Int DNW_ctrl(DEV_Handle device, Uns cmd, Arg parm)

en el nivel de red, el parámetro cmd de valor 1. El argumento parm es 0 para reservar sitio en Cyclic_buffer cuando se crea un canal de entrada y es 1 para liberarlo cuando el canal se destruye.


. Se ha dispuesto un mecanismo para establecer el time-out de un canal de entrada en cualquier instante (antes sólo se podía hacer en el momento en que el canal se creaba). Para ello se ha dispuesto para el mandato de control

   Int DNW_ctrl(DEV_Handle device, Uns cmd, Arg parm)

en el nivel de red, el parámetro cmd de valor 2. El argumento parm es el time-out en milisegundos.


. El servicio OPER_create ha cambiado su funcionalidad y su interfaz. Ahora toma parámetro "commSizes", un vector que contiene el calibre de sus comunicadores. OPER_create crea ahora los comunicadores (antes los creaba CHANNEL_create). Si no hay sitio en Cyclic_Buffer para los canales de entrada, OPER_create falla. Una segunda modificación es que OPER_create no arranca el operador. De ello se encarga ahora ua nueva primitiva del kernel: OPER_start.

  Int OPER_create(Idsp_Oper_t      *oper,  
                  Int               operCode,
                  Int               appCode,
                  Idsp_Oper_Addr_t  addr,
                  Int              *commSizes );



. La bibliotaca de canales se encuentra ahora con que los dos comunicadores subyacentes a cada canal ya existen y tienen un calibre o caudal dado. Para conocerlo se ha creado una nueva primitiva, COMM_getSize: 

  Int COMM_getSize(Int comm, Idsp_Oper_t operId) 



Si bien el atriibuto caudal de un comunicador viene dado en tiempo de creación del operador, no ocurre lo mismo con el atributo de time-out, que puede proporcionarse en tiempo de ejecución. Para ello se ha introducido una nueva primitiva de gestión de comunicadores, COMM_setTimeout:

  Int COMM_setTimeout (Int comm, Idsp_Oper_t operId, Uns timeout) 



. Se ha añadido la primitiva OPER_start. Toma un identificador de operador y crea la tarea DSP/BIOS en cuyo contexto ejecuta el operador.

  Int  OPER_start   (Idsp_Oper_t       oper, 
                     Idsp_Oper_Addr_t  addr      ); 



. El servicio GROUP_create ha cambiado su funcionalidad, de modo que ahora sólo es soportado por OPER_create. Una vez que el grupo ha sido creado, dispone de todos los recursos para comenzar a ejecutar. Entonces se invoca la nueva primitiva GROUP_start.



. Se ha añadido la primitiva GROUP_start: Toma un identificador de grupo y arranca las tarea DSP/BIOS en cuyo contexto ejecutan los operadores que componen el grupo.

Int GROUP_start  (Group_t  group);



. La interfaz del módulo de canales IDSP_CHANNEL_xx cambia a CHANNEL_xx. La primitiva CHANNEL_create ha cambiado, de modo que ahora no se especifica el calibre del canal: 

  Int CHANNEL_create (Idsp_Channel_t *ch, Uns mode, Uns Channel_Nr, Uns timeout); 



. Se han definido dos primitivas más. No añaden código, pero hacen más familiar IDSP al programador. Este debería limitarse a utilizar únicamente OPER_exit y OPER_kill y no utilizar OPER_destroy y OPER_disable. 

  #define OPER_exit()      OPER_destroy             \
                                   ( OPER_self(), \
                                    &OPER_self()->Address);
                                    
  #define OPER_kill        OPER_disable 



. Se ha añadido un método más al módulo ADDR2SPEC:

  Int ADDR2SPEC_removeSpecific(Int comm_addr);

Este método se invoca cuando se destruye un comunicador. Elimina la entrada del comunicador en la tabla de pares
[dirección del comunicador, (específico del SIO de entrada, específico del SIO de salida)]. Evita que llegue un paquete tardío a un comunicador que se acabe de destruir y se trate de encolar en un específico que realmente ya no existe.



. Los métodos OPER_xx se han extraído del kernel y se han dispuesto en el paquete "idsp_oper_svr.c". Son invocados por

  1. El kernel, cuando ejecuta el método de inicialización
  2. El servidor, hasta ahora denominado "IDSP_Server" y que ahora pasa a llamarse "OPER_service_loop", en el fichero fuente      "idsp_oper_skeleton.c" 

El kernel se reduce ahora a los métodos COMM_xx (más IDSP_init, el método de inicio). De este modo, su funcionalidad se reduce a la comunicación entre operadores. Desde este punto de vista, podemos decir que hemos definido una arquitectura microkernel, a saber:

  a) Un núcleo reducido que se limita a llevar a cabo el paso de mensajes
  b) Un par de servidores que llevan a cabo las funciones de gestionar operadores y grupos respectivamente.



. Se ha establecido con formalidad el concepto de "tamaño de un comunicador" como la longitud en bytes del campo "data" de un marco IDSP:

  /* Format of an IDSP packet: 
   *  ¦-------------¦-------------¦-------------¦-------------¦
   *  ¦ SRC address ¦ DST address ¦ DATA size   ¦    DATA     ¦
   *  ¦-------------¦-------------¦-------------¦-------------¦
   *
   *   <---------------- Header --------------->
   */

Como consecuencia, se ha introducido la primitiva CHANNEL_getSize:

#define \
     CHANNEL_getSize(ch) \
                    (ch)->Width  

Devuelve el "tamaño del canal", que es el mismo que el "tamaño del comunicador". Esta primitiva conlleva cambios en la programación de operadores de usuario. Por ejmplo, el operador de usuario "Source" consulta el tamaño del canal de salida antes de crear el buffer de datos que envía por dicho canal. El tamaño de este buffer es el mismo que toma el tamaño del canal.  


. El proyecto se ha estructurado utilizando subproyectos. IDSP es así más pequeño y manejable. Cada subproyecto construye una biblioteca que se enlaza al ejecutable final idsp_30. Las bibliotecas son:

  1. libchannel.lib
     Implementa los canales. Contiene el paquete "channel".

  2. libidsp.lib
     Implementa el núcleo y los dos servidores (operadores y grupos).  

  3. libcodec.lib
     Implementa los servicios necesarios para hacer uso del codec del DSK. Contiene los paquetes "tlc32ad535_codec" y
     "dsk_codec_stream".

  4. liblnk_mcbsp.lib
     Implementa el nivel de enlace de idsp que utiliza el puerto serie como dispositivo de comunicación. Contiene el
     paquete "lnk_mcbsp".

  5. libother.lib
     Implementa paquetes que deben ser usados tanto por el sistema como por los operadores de usuario. Contiene el 
     paquete "idsp_edma_isr"

  6. liboper.lib
     Implementa los operadores de usuario.


. Esta nueva estrutura impone tres nuevos subdirectorios en el directorio principal idsp_XX, a saber:

  1. include
     Contiene los ficheros .h de las bibliotecas:
       include/channel
       include/codec
       include/idsp
       include/lnk_mcbsp
       include/other

  2. lib
     Contiene los ficheros .c de las bibliotecas
       lib/channel
       lib/codec
       lib/idsp
       lib/lnk_mcbsp
       libe/other

  3. oper
     Contiene los ficheros .h y .c de los operadores de usuario

Esta es una estrategia para tener controlados los ficheros .h y facilitar la construcción de las bibliotecas. En concreto, especificamos estos directorios en Code Composer como sigue:
  
  Project -> Build Options... -> Compiler -> Preprocessor -> Include Search Path



. El núcleo (biblioteca libidsp.lib) se ha estructurado partiendose en tres. Desaparecen los directorios include/idsp y lib/idsp. Aparecen:

       include/kernel
       include/oper_server
       include/group_server

       lib/kernel
       lib/oper_server
       lib/group_server



. El operador admite un nuevo parámetro configurable en tiempo de compilación que es el tamaño de su pila. Para ello se ha actuado sobre el registro de operadores, el paquete OPER_REG_xx. Esto ha permitido disponer para el operador IVORY_OPERATOR una pila de 2048 octetos, en lugar de la habitual, 1024. Cuando se va a crear la tarea DSP/BIOS asociada al operador debemos proporcionar a TSK_create el tamaño de la pila en el parámetro de atributos. Para ello ha sido preciso añadir una nueva llamada al paquete:

  Int  OPER_REG_getStackSize (Int *stackSize, Int oper);



. Se ha mejorado la implementación de OPER_create en lo referente a la creación de los comunicadores del operador.



. Se ha introducido un nuevo operador de usuario: IVORY_OPERATOR.



. Se ha introducido una mejora considerable en el paquete DSK_CODEC_STREAM. Se han introducido dos nuevos métodos:

  Int DSK_CODEC_STREAM_init          (void);
  Int DSK_CODEC_STREAM_getCodecOffset(DSK_CODEC_STREAM_Stream_t  stream, Short  *offset);
 
El primero NO ES REENTRANTE, por lo que se invoca en la incialización del servidor de operadores. Se encarga de detectar el nivel de continua (offset) que introduce el codec del DSK, el tsl320ad535. Opera creando una corriente de entrad que extrae 200 ventanas de señal de audio. En esta señal no debe haber voz,sino quedebe reducirse al sonido ambiente. Mejor aún si la entrada de audio está desconectada. El segundo método extrae este nivel de continua.


. El operador IVORY_OPERATOR se ha modificado de modo que no confía en el paquete DSK_CODEC_STREAM para obtener la señal. Obtiene y emite esta a partir de un canal de entrada y otro de salida. No se percibe retraso alguno en el altavoz.


. El código fuente se ha reestructurado de nuevo. De ahora en adelante cada nuevo proyecto se estrutura en los directorios siguientes:

  idsp_35/
    algorithm/
      ivory            -- Fuentes de algoritmos matemáticos
      ...
    system/            -- Fuentes de isdp
      channel
      kernel
      oper_server
      group_server
      cio_server
      lnk_mcbsp
      other
    oper/              -- Fuentes de los operadores de usuario
      diarca
      microphone
      speaker
      source
      ...
    include/           -- .h
      system
    lib/               -- Repositorio de bibliotecas



. El operador IVORY_OPERATOR de idsp_34 ejecuta la secuencia de algoritmos :
    IVORY_espectro
    IVORY_plantilla
    IVORY_cuantizar
    IVORY_parser   

  IVORY_OPERATOR se ha partido en idsp_35 en dos operadores:
  1. DIARCA_SPECTRUM_OPERATOR
     Ejecuta el algortimo 
       IVORY_espectro

  2. IVORY_OPERATOR 
     Ejecuta la secuencia de algoritmos 
       IVORY_plantilla
       IVORY_cuantizar
       IVORY_parser   


. Se ha introducido la aplicación DIARCA_APP, de cuatro operadores

  | 1 | -> | 2 | -> | 3 | 
    |
    V 
  | 4 |

  donde:
    1 es MICROPHONE_OPERATOR
    2 es DIARCA_SPECTRUM_OPERATOR
    3 es IVORY_OPERATOR 
    4 es SPEAKER_OPERATOR

  Observemos que el canal de salida 0 de MICROPHONE_OPERATOR tiene un fan-out de 2, emitiendo la señal del codec tanto a DIARCA_SPECTRUM_OPERATOR como a SPEAKER_OPERATOR.




. La aplicación DIARCA_APP tiene ahora cinco operadores. DIARCA_TEMPLATE_OPERATOR se ha extraido de IVORY_OPERATOR:

  | 1 | -> | 2 | -> | 3 | -> | 4 | 
    |        |                 ^
    V        |_________________|
  | 5 |

  donde:
    1 es MICROPHONE_OPERATOR
    2 es DIARCA_SPECTRUM_OPERATOR
    3 es DIARCA_TEMPLATE_OPERATOR
    4 es IVORY_OPERATOR 
    5 es SPEAKER_OPERATOR




. La aplicación DIARCA_APP tiene ahora SEIS operadores. IVORY_OPERATOR se ha distribuido así completamente sobre IDSP:

  | 1 | -> | 2 | -> | 3 | -> | 4 | -> | 5 | 
    |        |                          ^
    V        |__________________________|
  | 6 |

  donde:
    1 es MICROPHONE_OPERATOR
    2 es DIARCA_SPECTRUM_OPERATOR
    3 es DIARCA_TEMPLATE_OPERATOR
    4 es DIARCA_QUANTIZATION_OPERATOR
    5 es DIARCA_PARSER_OPERATOR
    6 es SPEAKER_OPERATOR



. OPER_upgrade y OPER_degrade se han terminado, de modo que un número arbitrario y simultáneo de tareas DSP/BIOS pueden promocionar a operadores IDSP.



. La gestión de grupos se ha simplificado y su interfaz ha cambiado ligeramente:

  Int     GROUP_create   (Int *gix, Int appCode, Int commSizes[][MAX_COMMUNICATOR]);
  Int     GROUP_destroy  (Int  gix);
  Int     GROUP_start    (Int  gix);
  Int     GROUP_getFanOut(Int  gix, Fanout *fanout, Idsp_Comm_Addr_t commAddr);
  #define GROUP_self()   OPER_getGix()
 


. Corregido un error de incialización en la inicialización del paquete APP_REG, que se manifestaba esporádicamente.



. Cambio en el operador DIARCA_spectrum (DIARCA_spectrum.c) para soportar el solapamiento de ventanas. Ahora se generan dos espectros de 128 canales por cada ventana de 256 muestras. 



. Cambio en el driver LNK_MCBSP. La función que extrae paquetes de Cyclic_Buffer ejecutaba antes en el contexto de una PRD DSP/BIOS establecida estáticamente.  Ahora desaparece de idsp.cdb y pasa a ejecutar en el contexto de una función periódica que arranca la rutina de incialización de LNK_MCBSP. No hay degradación perceptible en la carga de UCP.



. El paquete DSK_CODEC_STREAM desaparece. Se parte ahora en dos paquetes: STREAM_CODEC y STREAM_CODEC_TSL320AD535. El primero es independiente del dispositivo y utiliza los servicios del segundo, que soporta hasta 4 codecs TSL320AD535. Sus definiciones de interface son

  Int STREAM_CODEC_init          (void);
  Int STREAM_CODEC_create        (STREAM_CODEC_Stream_t *stream, Int ioMode, Int itemDim);
  Int STREAM_CODEC_destroy       (STREAM_CODEC_Stream_t  stream);
  Int STREAM_CODEC_get           (STREAM_CODEC_Stream_t  stream, Short **item);
  Int STREAM_CODEC_put           (STREAM_CODEC_Stream_t  stream, Short  *item);
  Int STREAM_CODEC_setDev        (STREAM_CODEC_Stream_t  stream, Int codec, Int mcbsp);

y 

  Int STREAM_CODEC_TLC320AD535_init       (Void);
  Int STREAM_CODEC_TLC320AD535_initHw     (STREAM_CODEC_Stream_t stream);
  Int STREAM_CODEC_TLC320AD535_terminateHw(STREAM_CODEC_Stream_t stream);
  Int STREAM_CODEC_TLC320AD535_getOffset  (STREAM_CODEC_Stream_t stream, Short  *offset);

respectivamente.



. El paquete STREAM_CODEC_TLC320AD535 sufre cambios significativos y pasa a redenominarse como STREAM_CODEC_LOW. El sufijo LOW viene a decir que es la continuación a bajo nivel de STREAM_CODEC. STREAM_CODEC_LOW es ahora completamente independiente del codec TLC320AD535. ESte se trata en STREAM_CODEC_LOW como un caso particular. El interface de STREAM_CODEC_LOW es

  Int STREAM_CODEC_LOW_init     (Void);
  Int STREAM_CODEC_LOW_plug     (STREAM_CODEC_Stream_t stream);
  Int STREAM_CODEC_LOW_unplug   (STREAM_CODEC_Stream_t stream);

La filosofía subyacente estriba en asociar un descriptor a cada uno de los Mcbsp del C6000. Cada descriptor es inicializado en STREAM_CODEC_LOW_init según las caractrísticas de transmisión del Codec subyacente. Una vez inicializado un Mcbsp, podemos "enchufar" al mismo dos canales de EDMA, uno para entrada y otro para salida. De ello se encarga STREAM_CODEC_LOW_plug. Al nivel más alto, STREAM_CODEC_create especifica un codec en su cuarto parámetro e invoca STREAM_CODEC_plug, que inicializará el Mcbsp según las necesidades del codec.  

El paquete STREAM_CODEC cambia un poquito su interfaz, simplificándola. Notese que desaparece el parámetro McBsp de la primitiva STREAM_CODEC_create, quedando sólo el código del codec, muy parecida a una primitiva "open" de UNIX:

  Int STREAM_CODEC_init          (void);
  Int STREAM_CODEC_create        (STREAM_CODEC_Stream_t *stream, Int ioMode, Int itemDim, Int codec);
  Int STREAM_CODEC_destroy       (STREAM_CODEC_Stream_t  stream);
  Int STREAM_CODEC_get           (STREAM_CODEC_Stream_t  stream, Short **item);
  Int STREAM_CODEC_put           (STREAM_CODEC_Stream_t  stream, Short  *item);
  Int STREAM_CODEC_getOffset     (STREAM_CODEC_Stream_t  stream, Short *offset);

 

. El paquete EDMA_ISR cambia. Desacoplamos la obtención del TCC y la asignación de la correspondiente rutina de interrupción. Su interfaz es ahora:

  void EDMA_ISR_isr       (void);
  void EDMA_ISR_init      (void);
  Int  EDMA_ISR_allocTcc  (Int *tcc);
  Int  EDMA_ISR_freeTcc   (Int  tcc);
  Int  EDMA_ISR_register  (Int  tcc, Fxn function, Int p1, Int p2);
  Int  EDMA_ISR_unregister(Int  tcc);



. El paquete STREAM_CODEC_LOW desaparece. Aparece el paquete EDMA_PORT, un driver para McBSP con dos canales de EDMA.

  Void EDMA_PORT_init            ();
  Void EDMA_PORT_read            (Int mcbspNr, Short *dst, Int nsamps);
  Void EDMA_PORT_write           (Int mcbspNr, Short *src, Int nsamps);
  Int  EDMA_PORT_setReadNotify   (Int mcbspNr, Fxn upcall, Int param_1, Int param_2);
  Int  EDMA_PORT_unsetReadNotify (Int mcbspNr);
  Int  EDMA_PORT_setWriteNotify  (Int mcbspNr, Fxn upcall, Int param_1, Int param_2);
  Int  EDMA_PORT_unsetWriteNotify(Int mcbspNr);
  Int  EDMA_PORT_getOffsetLevel  (Int mcbspNr);


De momento sólo sirve para muestras de 16 bits, pero eso puede cambiar. Read y write con asíncronas. Cuando EDMA_PORT_write retorna, los datos aún no han salido en su totalidad del puerto serie. Cuando este evento se produce, puede ejecutarse una rutina que comunica este hecho. Esta rutina se establece mediante EDMA_PORT_setWriteNotify. El caso de la lectura es equivalente. Cuando EDMA_PORT_read retorna, los datos solicitados no están en su totalidad en su destino. Este evento se produce cuando se ejecuta la función de notificación, establecida de forma discreccional mediante EDMA_PORT_setReadNotify. 



. La operación del codec AD535 se ha hecho más robusta. Se ha puesto atención en la inicialización del EDMA justo tras main(). Esta inicialización se hace tras la ejecución de main(), concretamente en EDMA_ISR_init. Así esta función debe ser la primera que se invoca tras la ejecución de main.



. IDSP soporta ahora una tarjeta hija del DSK. Es la tarjeta de audio "Audio Daughter Card - TMDX326040A" (ftp://ftp.ti.com/pub/cs/c6x/DSK/AudioDC). Para utilizarla es preciso definir la constante AUDIO_DAUGHTER_CARD_TMDX326040A en el fichero "idsp_config.h". Esta tarjeta dispone del codec PCM3003, por lo que los operadores pueden crear un stream a este codec mediante una invocación como 

  STREAM_CODEC_create(&InputStream, STREAM_CODEC_INPUT, windowDim, PCM3003);



. El paquete STREAM_CODEC ha sido reimplementado. Su interfaz es ahora:

  Int STREAM_CODEC_create        (STREAM_CODEC_Stream_t *stream, Int ioMode, Int bufferDim, Int windowDim, Int codec);
  Int STREAM_CODEC_destroy       (STREAM_CODEC_Stream_t  stream);
  Int STREAM_CODEC_get           (STREAM_CODEC_Stream_t  stream, Int **window_0, Int **window_1);
  Int STREAM_CODEC_put           (STREAM_CODEC_Stream_t  stream, Int  *window_0, Int  *window_1);
  Int STREAM_CODEC_getOffset     (STREAM_CODEC_Stream_t  stream, Int *offset);

Como puede observarse, el método de inicialización STREAM_CODEC_init desaparece. En STREAM_CODEC_create aparece el nuevo parámetro bufferDim, que se sirve para dimensionar el buffer acotado. Un buen valor es dos. El paquete reconoce ahora la existencia de codecs estéreo. De ahí la distinción entre window_0, para el canal izquierdo y window_1 para el canal derecho. En el caso de codecs mono, window_1 debe ser NULL en STREAM_CODEC_put y ser ingnorado en STREAM_CODEC_get. 



. El paquete EDMA_PORT ha sido reimplementado aplicando una capacidad del hardware EDMA: El enlace de canales. En segundo lugar, el paquete es ahora independiente del o de los codecs asignados a los MCBSPs. Además, ha cambiado de nombre, pasando a llamarse VPORT para dar más énfasis a su carácter de dispositivo MCBSP extendido o "puerto virtual". Su interfaz es ahora:

  Void VPORT_init             ();
  Int  VPORT_open             (VPORT_Port_t *port, Int  McbspNr, Int ioMode);
  Int  VPORT_close            (VPORT_Port_t  port);
  Void VPORT_start            (VPORT_Port_t  port);
  Void VPORT_stop             (VPORT_Port_t  port);
  Int  VPORT_setBuffer        (VPORT_Port_t  port, Int *buffer,  Int blockMax, Int blockDim);
  Int  VPORT_setNotify        (VPORT_Port_t  port, Fxn  upcall,  Int param_1,  Int param_2);
  Int  VPORT_unsetNotify      (VPORT_Port_t  port);
  Int  VPORT_getTcc           (VPORT_Port_t  port);
  Int  VPORT_getMcbsp         (VPORT_Port_t  port, MCBSP_Handle *hMcbsp);
  Int  VPORT_configMcbsp      (VPORT_Port_t  port, MCBSP_Config *mcbspConfig);

Las nuevas primitivas VPORT_getMcbsp y VPORT_configMcbsp implementan la transparencia al CODEC. El nivel superior, sea STREAM_CODEC u otro conoce el Codec y se ocupa de configurar el McBSP de forma adecuada utilizando la primitiva VPORT_configMcbsp. La primitiva VPORT_getMcbsp ha sido necesaria implementarla porque la necesitaba el paquete STREAM_CODEC para inicializar el codec TLC320AD535.



. El fichero "idsp_net" del kernel ha sido modificado de forma que no se enlace el paquete "LNK_MCBSP" cuando sólo existe paso de mensajes síncrono. Este caso puede darse en una configuración de una única máquina. 



. El método VPORT_start del paquete VPORT cambia de 
  
    Void VPORT_start(VPORT_Port_t  port);
a
    Int  VPORT_start(VPORT_Port_t  port);

La razon es que el método no retornaba nunca si no habia un codec conectado al Mcbsp. Este caso se daba, por ejemplo, si la tarjeta de audio no estaba insertada en el DSK. Ahora VPORT devuelve -1 si no hay codec.



. 11/Marzo/03. 
  El paquete LNRADDR2QUEUE se ha reimplementado. Las direcciones lineales han desaparecido, ya que el concepto era demasiado restrictivo. El paquete se denomina ahora ADDR2QUEUE. Confía en el paquete GMAP, que a su vez confia en el paquete OMAP que a su vez confia en el paquete CMAP. El módulo "idsp_addr2queue.h" contiene una documentación razonable sobre su implementación.



. 15/Marzo/03. 
  El número de grupos operando simultáneamente es ahora ilimitado. Como consecuencia, la constante G_MAX ya no reside en idsp_config.h, sino en la implementación de GROUP_REG. La idea es que ya no es un parámetro relevante para el usuario de IDSP. El número de operadores por grupo, O_MAX, que estaba limitado anteriormente, es ahora también prácticamente ilimitado. Idsp opera sin problemas con un valor de O_MAX de 1024.



. 16/Marzo/03. 
  La interfaz de GROUP_REG mejora. Pasa de 

  void    GRP_REG_init     (void);
  Int     GRP_REG_getGid   (Group_t *gid, Int gix);
  Int     GRP_REG_add      (Group_t  gid         );
  Int     GRP_REG_allocGix (Int     *gix         );
  Int     GRP_REG_freeGix  (Int      gix         );

a

  Void    GROUP_REG_init     (Void);
  Void    GROUP_REG_insert   (Group_t  gid         );
  Void    GROUP_REG_remove   (Group_t  gid         );
  Int     GROUP_REG_allocGix (Int     *gix         );
  Void    GROUP_REG_freeGix  (Int      gix         );
  Int     GROUP_REG_getGid   (Group_t *gid, Int gix);



. 20/Marzo/03. 
  La interfaz del paquete GROUP mejora. Se cambia el nombre de GROUP_destroy por de GROUP_kill, que es más significativo.  

  Int     GROUP_create   (Int *gix, Int appCode, Int commSizes[][P_MAX]);
  Int     GROUP_destroy  (Int  gix);
  Int     GROUP_start    (Int  gix);
  Int     GROUP_getFanOut(Int  gix, Fanout *fanout, Idsp_Comm_Addr_t commAddr);
  #define GROUP_self()   OPER_getGix()

a 

  Int     GROUP_create   (Int *gix, Int appCode, Int commSizes[][P_MAX]);
  Void    GROUP_kill     (Int  gix);
  Int     GROUP_start    (Int  gix);
  Void    GROUP_leave    (Void);
  Int     GROUP_getFanOut(Int  gix, Fanout *fanout, Idsp_Comm_Addr_t commAddr);
  Int     GROUP_self     (Void);


Se ha implementado la nueva primitiva GROUP_leave, que da de baja el operador en el grupo. Cuando un operador termina, debería invocar GROUP_leave y OPER_exit a continuación. Si no se hace así, el grupo queda en estado de ZOMBIE.



. 22/Marzo/2003
  Se ha eliminado una copia en la cita de dos tareas (Una que hace COMM_send y la otra COMM_receive). Esto hace a IDSP mucho más rápido que antes, lógicamente, pero no se han hecho medidas. La mejora sólo funciona en cita de dos tareas locales. IDSP_47 se cuelga para tareas que comunican vía red. Queda pendiente la terminación para IDSP_48.



. 29/Marzo/03
  Durante esta semana se han mejorado las especificaciones ".h" de varios paquetes. Para ello se han ocultado en los ".c" las implementaciones de los tipos opacos que exportan dichas especificaciones. Por ejemplo, en la versión anterior, idsp_46, la definición del tipo celda era:

  struct Idsp_Cell {
    QUE_Elem       Cell_Link;            
    QUE_Handle	   Oper_Queue;  
    Addr_Pair      Pair[O_MAX];
    Int            Gix;
  }; 
  typedef struct Idsp_Cell Idsp_Cell, *Idsp_Cell_t;

en el fichero "idsp_cell.h". Ahora, sin embargo, "idsp_cell.h" simplemente exporta:

  typedef struct Idsp_Cell Idsp_Cell, *Idsp_Cell_t;

quedando Idsp_Cell_t como un verdadero tipo opaco. Su definición

  struct Idsp_Cell {
    QUE_Elem       Cell_Link;            
    QUE_Handle	   Oper_Queue;  
    Addr_Pair      Pair[O_MAX];
    Int            Gix;
  }; 

se desplaza a la implementación del paquete (el fichero "idsp_cell.c"). Esta mejora en la especificación, sin embargo, hace que falle el uso del paquete. Ello ha conllevado reescribir este uso. La labor merece la pena ya que ahora el código cliente no ve en absoluto la implementación, con lo que aumenta su calidad. El hecho de no ver la implementación del tipo ha motivado en ocasiones una extensión de la interfaz. Por ejemplo, la interfaz del paquete CELL se extiende con la primitiva

 Int  CELL_getLocInfo(Idsp_Cell_t cell, Int *mix, Int oix);

que determina la máquina en la ejecuta un determinado operador de un grupo.


. 1/Abril/03 
  Todos los tipos exportados por los paquetes son ahora opacos. Esto ha ocasionado que el código aumente ligeramente de tamaño. Se ha resuelto definitivamente el hecho de que el kernel y el servidor de operadores "compartieran" el descriptor de operador: El kernel hace "upcalls" al servidor de operadores. Paso a IDSP_48


. 22/Mayo/03. Idsp_48
Idsp_48 destierra el código antiguo de la CSL (Chip Support Library). Esto supone que ya no es necesario hacer el 
  
  #include <csl_legacy.h>

en los ficheros que trabajan con los periféricos del C6711.  



. 10/Junio/03. Idsp_49
El paradigma de comunicación basado en un SIO se ha desestimado por ineficiente. Como consecuencia, desaparece el fichero idsp_net.c. El cambio conlleva una mejora de rendimiento y una disminución muy significativa del ejecutable (de 103 Kb a 73 Kb).

El mencionado cambio, desgraciadamente, llega a un cambio en el interface COMM_XX, que se detallan a continuación:

  Int COMM_send      (Int          cix,  Idsp_Comm_Addr   *dst,  Ptr  buffer, Int  nbytes);  
  Int COMM_receive   (Int          cix,  Idsp_Comm_Addr   *src,  Ptr  buffer, Int *nbytes); 
  Int COMM_sendrec   (Int          cix,  Idsp_Comm_Addr   *addr, Ptr  buffer, Int *nbytes); 

El cambio supone la desparición del concepto de "tamaño" del comunicador, por lo que las primitivas

  COMM_getSize
  COMM_setSize

simplemente desaparecen.

A su vez, el cambio lleva a un cambio en la interfaz de canal. La primitiva

  Int  CHANNEL_getSize(Channel_t  ch)

desaparece. El cambio ha su puesto, de igual modo, la modificación de los operadores de usuario.

Otro cambio importante en idsp_49 es que el objeto operador Idsp_Oper ya NO es el entorno de la tarea DSP/BIOS. Esto debe suponer que sea más fácil promocionar a operador a una tarea DSP/BIOS cualquiera no programada para IDSP. Utilizamos para ello el concepto de HOOK DSP/BIOS. A partir de ahora, todo ejecutable IDSP debe disponer del Hook OPER_hook en su fichero de configuración. El cambio, no obstante, no afecta a la interface OPER_XX. La primitiva OPER_self se ha reimplementado, pero su uso no cambia.


. 15/Junio/03. Idsp_50
El módulo LNK_MCBSP, que no funcionaba en IDSP_49, ya lo hace. Hay cambios tanto en la interfaz como en la implementacion. La primitiva LNK_MCBSP_put se ha vectorizado.

 Int  LNK_MCBSP_put        (Vbuffer *vector, Int vcnt);


. Ahora se pueden pasar parámetros a los operadores. El operador admite un único parámetro que es un puntero a una estructura. La estructura se almacena en el objeto operador cuando este se crea, de modo que no puede ser muy grande. Tiene un máximo de PARAM_SIZE bytes, constante definida en idsp_config.h. Como consecuencia, cambian las primitivas OPER_create y GROUP_create. OPER_create ahora toma un parámetro que consiste en la dirección de una estructura que coincide con el parámetro del operador que crea. GROUP_create toma un vector de estos parámetros. 


. El kernel se ha aumentado con la primitiva COMM_init. Esto simplifica el procedimiento de inicialización IDSP_init, haciéndolo más intuitivo y claro.


. 18/Junio/03. Idsp_51
Desaparece el fichero "idsp_msg.h", que se reparte entre "group_msg.h" y "oper_msg.h". El significado de este cambio es que los mensajes del sistema son ahora mucho más pequeños, algunos réplicas de un único entero. Antes de este cambio todos los mensajes del sistema eran del mismo tamaño, el del mensaje más grande, a saber, 540 octetos. El resultado es una ganancia en la agilidad del sistema.


. 27/Junio/03. Idsp_52
Se ha introducido una nueva primitiva de comunicación:

  Int COMM_asend(Int cix, COMM_Addr *dst,  char *buffer, Int  nbytes);  

Como se aprecia, es igual que COMM_send. La diferencia estriba en que COMM_asend es NO bloqueante. COMM_asend ha venido motivada porque es la primitiva de envío que utilizan todos los DSP/RTOS comerciales como OSE, Diamond, Virtuoso, etc. En IDSP_52 TODAS las llamadas COMM_send se han sustituido por COMM_asend. El resultado es una mejora en el rendimiento, que se muestra en el siguiente experimento: 

  Se han lanzado cuatro ejecuciones de dos instancias de ADDER_APPLICATION según el patrón de uso del operador "dispatcher" (en "dispatcher.c"). Pues bien:
  a) Con CHANNEL_send en modo síncrono se han medido 1624 ejecuciones del operador ADDER_OPERATOR
  b) Con CHANNEL_send en modo asíncrono Se han medido 2108 ejecuciones del operador ADDER_OPERATOR

Esto significa una mejora del rendimiento para este caso particular del 29,8%, algo realmente impresionante.


. 12/Julio/03. Idsp_53
Esta versión es el comienzo de un proyecto: Hacer que la interfaz de IDSP se la parezca lo más posible a la interfaz de envío y recepción de mensajes de MPI. Se han creado primitivas del kernel que soportan el envío y la recepción asíncronos. La interfaz del kernel es ahora:

  Void COMM_init      (Void); 
  Int  COMM_create    (COMM_Comm_t *comm, const COMM_Attr_t attr, COMM_Addr_t addr, Ptr operId);                           
  Void COMM_destroy   (COMM_Comm_t  comm); 

  Int  COMM_send      (Int cix, char *buffer, Int  nbytes, COMM_Addr_t dst, Int tag);  
  Int  COMM_recv      (Int cix, char *buffer, Int *nbytes, COMM_Addr  *src, Int tag); 
  Int  COMM_asend     (Int cix, char *buffer, Int  nbytes, COMM_Addr_t dst, Int tag, COMM_Rqst *rqst);  
  Int  COMM_arecv     (Int cix, char *buffer, Int *nbytes, COMM_Addr  *src, Int tag, COMM_Rqst *rqst); 
  Int  COMM_wait      (Int cix, COMM_Rqst *rqst);  
  Int  COMM_test      (Int cix, COMM_Rqst *rqst, Bool *flag);  
  Int  COMM_sendrec   (Int cix, char *buffer, Int *nbytes, COMM_Addr *addr); 
  Int  COMM_setTimeout(Int cix, Uns   timeout); 
  Int  COMM_deliverMsg(COMM_MsgHdr_t header, Int mode, COMM_Comm_t srcComm, Int rqst);

Las pruebas realizadas muestran que la nueva funcionalidad reduce la velocidad de idsp en un 0.25%.

. La nueva interfaz del kernel ha posibilitado enriquecer la interfaz de canal con primitivas asíncronas. La nueva versión es 

  Int  CHANNEL_create (Channel_t *ch, Uns ioMode,   Uns  channelNr, Uns  timeout); 
  Void CHANNEL_destroy(Channel_t  ch); 
  Int  CHANNEL_send   (Channel_t  ch, char *buffer, Int  nbytes);
  Int  CHANNEL_recv   (Channel_t  ch, char *buffer, Int *nbytes);
  Int  CHANNEL_asend  (Channel_t  ch, char *buffer, Int  nbytes, CHANNEL_Rqst *rqst);
  Int  CHANNEL_wait   (Channel_t  ch, CHANNEL_Rqst *chRqst); 
  Int  CHANNEL_test   (Channel_t  ch, CHANNEL_Rqst *chRqst, Bool *flag); 

El tamaño de IDSP es ahora, sin contar el módulo LNK_MCBSP de 32 Kb.


. 09/Septiembre/03. Idsp_54
Esta es una versión con cambios importantes, fundamentalmente debidos al objetivo de soporte de MPI. La versión idsp_53 realmente no soportaba la recepción asíncrona. IDSP_54 sí lo hace. La velocidad de IDSP no ha variado, pero su tamaño ha aumentado. Se ha introducido un nuevo paquete en el núcleo, "idsp_rqst". Idsp_rqst implementa el concepto de petición de comunicación, que ha surgido de forma natural en el trabajo sobre el kernel y, de paso, sirve para soportar el concepto de petición MPI, presente en las primitivas asíncronas MPI_Isend, MPI_Irecv, MPI_wait, etc. Con ello, la interfaz del núcleo es ahora la siguiente.  

  Void COMM_init      (Void); 
  Int  COMM_create    (COMM_Comm_t *comm, const COMM_Attr_t attr, ADDR_Addr_t addr, Ptr operId);
  Void COMM_destroy   (COMM_Comm_t  comm); 
  Int  COMM_setTimeout(COMM_Comm_t  comm, Uns   timeout); 
  Int  COMM_sendrec   (char        *buffer, Int count, ADDR_Addr_t addr); 
  Int  COMM_deliverMsg(RQST_MsgDptr_t msgDptr, Bool *delivered);

  Int  COMM_send      (char        *buffer, Int count, ADDR_Addr_t dst, Int tag);  
  Int  COMM_asend     (char        *buffer, Int count, ADDR_Addr_t dst, Int tag,          COMM_Rqst_t *rqst);  
  Int  COMM_recv      (char        *buffer, Int count, ADDR_Addr_t src, Int tag, Int cix, COMM_Status *status); 
  Int  COMM_arecv     (char        *buffer, Int count, ADDR_Addr_t src, Int tag, Int cix, COMM_Rqst_t *rqst); 
  Int  COMM_wait      (COMM_Rqst_t  rqst,                                                 COMM_Status *status);  
  Int  COMM_test      (COMM_Rqst_t  rqst,   Int *flag,                                    COMM_Status *status);  

Como se puede observar las últimas 6 llamadas son muy parecidas a sus correspondientes MPI MPI_send, MPI_Isend, etc. Desde luego, falta por diseñar e implementar el concepto de "comunicador" MPI, un objetivo para posteriores versiones.

. La nueva interfaz de canal permite ahora recepción asíncrona: 

  Int  CHANNEL_create (Channel_t *ch, Uns mode,     Uns   channelNr, Uns  timeout); 
  Void CHANNEL_destroy(Channel_t  ch); 
  Int  CHANNEL_send   (Channel_t  ch, char *buffer, Int  nbytes);
  Int  CHANNEL_recv   (Channel_t  ch, char *buffer, Int *nbytes);
  Int  CHANNEL_arecv  (Channel_t  ch, char *buffer, Int *nbytes, CHANNEL_Rqst *rqst);
  Int  CHANNEL_asend  (Channel_t  ch, char *buffer, Int  nbytes, CHANNEL_Rqst *rqst);
  Int  CHANNEL_wait   (CHANNEL_Rqst *chRqst); 
  Int  CHANNEL_test   (CHANNEL_Rqst *chRqst, Bool *flag); 

. La implementación se ha simplificado eliminando los semáforos asociados a los comunicadores. Ahora sólo existe un semáforo por operador, que se utiliza para suspenderlo cuando se invoque una primitiva bloqueante.



. 11/Septiembre/03. Idsp_55
Idsp_55 aporta nueva funcionalidad. Introduce la posibilidad de bloqueo en más de un comunicador (una especie de "select" UNIX) al estilo MPI. Concretamente aparecen las funciones COMM_waitany y COMM_waitall. COMM_wait se ha reimplementado en IDSP_55 como un caso particular de COMM_waitany. COMM_waitall se ha implementado sobre COMM_wait.

Aparecen CHANNEL_waitany y CHANNEL_waitall en la biblioteca de canales (Nota: Estas sólo se han implementado en IDSP_55 sobre canales de salida). A pesar del crecimiento, IDSP no ha perdido rapidez. Tiene un tamaño de 48 Kb.

  Void COMM_init      (Void); 
  Int  COMM_create    (COMM_Comm_t *comm, const COMM_Attr_t attr, ADDR_Addr_t addr, Ptr operId);
  Void COMM_destroy   (COMM_Comm_t  comm); 
  Int  COMM_setTimeout(COMM_Comm_t  comm, Uns   timeout); 
  Int  COMM_sendrec   (char        *buffer, Int count, ADDR_Addr_t addr); 
  Int  COMM_deliverMsg(RQST_MsgDptr_t msgDptr, Bool *delivered);

  Int  COMM_send      (char        *buffer, Int count, ADDR_Addr_t dst, Int tag);  
  Int  COMM_recv      (char        *buffer, Int count, ADDR_Addr_t src, Int tag, Int cix, COMM_Status *status); 
  Int  COMM_asend     (char        *buffer, Int count, ADDR_Addr_t dst, Int tag,          COMM_Rqst_t *rqst);  
  Int  COMM_arecv     (char        *buffer, Int count, ADDR_Addr_t src, Int tag, Int cix, COMM_Rqst_t *rqst); 
  Int  COMM_wait      (COMM_Rqst_t  rqst,                                                 COMM_Status *status);  
  Int  COMM_test      (COMM_Rqst_t  rqst,   Int *flag,                                    COMM_Status *status);  
  Int  COMM_waitall   (Int          count,  COMM_Rqst_t *rqst,                            COMM_Status *status[]);
  Int  COMM_waitany   (Int          count,  COMM_Rqst_t *rqst, Int *index,                COMM_Status *status);

La interfaz de canal es ahora, pues

  Int  CHANNEL_create (Channel_t *ch, Uns mode,     Uns   channelNr, Uns  timeout); 
  Void CHANNEL_destroy(Channel_t  ch); 
  Int  CHANNEL_send   (Channel_t  ch, char *buffer, Int  nbytes);
  Int  CHANNEL_recv   (Channel_t  ch, char *buffer, Int *nbytes);
  Int  CHANNEL_arecv  (Channel_t  ch, char *buffer, Int *nbytes, CHANNEL_Rqst *rqst);
  Int  CHANNEL_asend  (Channel_t  ch, char *buffer, Int  nbytes, CHANNEL_Rqst *rqst);
  Int  CHANNEL_wait   (CHANNEL_Rqst *chRqst); 
  Int  CHANNEL_test   (CHANNEL_Rqst *chRqst, Bool *flag); 
  Int  CHANNEL_waitall(Int count, CHANNEL_Rqst *chRqst); 
  Int  CHANNEL_waitany(Int count, CHANNEL_Rqst *chRqst, Int *index);


. 15/Septiembre/03. Idsp_56
Se ha completado la implementación de CHANNEL_waitany y CHANNEL_waitall para canales de entrada. CHANNEL_wait se ha implementado sobre CHANNEL_waitany, que ha sido rediseñado. 

Idsp_config.h se ha simplificado, lo que hace a idsp más fácil de comprender y sintonizar. 

Se ha reparado un fallo en el paquete idsp_rqst.

IDSP_56 ocupa poco más 37 Kb.



. 09/Octubre/03. Idsp_57
Para empezar, en Idsp_57 desaparece el concepto de "aplicación". En consecuencia, desaparece el módulo APP_REG del servidor de grupos, que especificaba cómo se combinaba un grupo de operadores a través de sus canales para formar una aplicación dada con un nombre bien conocido. Esta aproximación tiene una debilidad patente: Son muchas las formas de combinar los operadores, casi infinitas, y no es posible darles un nombre a todas. En consecuencia, el programador invoca ahora GROUP_create pasando como parámetro un vector "oper" de "size" nombres de operadores en lugar de un código de aplicación. A continuación,m la nueva llamada "GROUP_channel" combina los canales del grupo pasando como parámetro dos matrices de "size x size". La primera de ellas especifica los operadores fuente de los canales y la segunda los operadores destino (Ver los comentarios de la función, implementada en el fichero "idsp_group_svr.c", que definen las matrices). La nueva interfaz de grupo es ahora:

  Void GROUP_init     (Void); 
  Int  GROUP_create   (Int *gix, Int mode, Int *oper, Void **param, Int size);
  Int  GROUP_destroy  (Int  gix);
  Int  GROUP_channel  (Int  gix, Int *inCh, Int *outCh, Int size);     <-- ¡nueva!
  Int  GROUP_kill     (Int  gix);!
  Int  GROUP_start    (Int  gix);
  Void GROUP_leave    (Void);
  Int  GROUP_self     (Void);

La creación de un operador, "OPER_create", se simplifica, ya que ahora no es preciso especificar la aplicación de la que forma parte. Por supuesto, desaparece "OPER_getApp". La contrapartida de la desparición del concepto de aplicación es la apareción de las primitivas "OPER_setChannel", "OPER_getSources" y "OPER_getSinks", utilizadas por la biblioteca de canal.

La segunda modificación importante estriba en que ahora todos los canales de un operador utilizan un comunicador común y único. Esto desliga el canal de los recursos que lo implementan. El envío se hace ahora al OPERADOR y no al par (OPERADOR, COMUNICADOR). En consecuencia, todas las direcciones a las que se hace un envío llevan el campo CIX igual a cero. Los envíos de CHANNEL_send portan ahora etiquetas, que especifican el canal de entrada del operador destino, sea el 0, 1, 2, ... 

La tercera modificación se ha realizado en los plazos. Hasta ahora, el plazo se había asociado al comunicador, por lo que la primitiva COMM_recv no necesitaba un parámetro de plazo. Esta aproximación era inflexible y traía problemas que se han manifestado en algunos operadores como el MONO_SPEAKER, donde la primera invocación de COMM_recv precisaba de un plazo mucho más grande que el resto, esperando que llegase la primera trama de voz. Por otra parte, la introducción de un único comunicador por operador invalidada esta aproximación, ya que impondría que todos los canales del operador deberían tener el mismo plazo. En la versión Idsp_57 se ha eliminado, pues, la primitiva "COMM_setTimeout" y se ha añadido a "COMM_recv" un parámetro de plazo. COMM_create se simplifica, prescindiento de los atributos del communicador que se crea. La nueva interfaz es: 

 Void COMM_init      (Void); 
 Int  COMM_create    (COMM_Comm_t *comm, ADDR_Addr_t addr, Ptr operId);
 Void COMM_destroy   (COMM_Comm_t  comm); 
 Int  COMM_sendrec   (char        *buffer, Int count, ADDR_Addr_t addr); 
 Int  COMM_deliverMsg(RQST_MsgDptr_t msgDptr, Bool *delivered);

 Int  COMM_send      (char        *buffer, Int count, ADDR_Addr_t dst, Int tag);  
 Int  COMM_recv      (char        *buffer, Int count, ADDR_Addr_t src, Int tag, Int cix, COMM_Status *status, Int timeout); 
 Int  COMM_asend     (char        *buffer, Int count, ADDR_Addr_t dst, Int tag,          COMM_Rqst_t *rqst);  
 Int  COMM_arecv     (char        *buffer, Int count, ADDR_Addr_t src, Int tag, Int cix, COMM_Rqst_t *rqst); 
 Int  COMM_wait      (COMM_Rqst_t  rqst,                                                 COMM_Status *status);  
 Int  COMM_test      (COMM_Rqst_t  rqst,   Int *flag,                                    COMM_Status *status);  
 Int  COMM_waitall   (Int          count,  COMM_Rqst_t *rqst,                            COMM_Status *status[]);
 Int  COMM_waitany   (Int          count,  COMM_Rqst_t *rqst, Int *index,                COMM_Status *status);


La interfaz de canal cambia, también, pero sólo muy ligeramente. Observemos que el parámetro "time-out" se desplaza desde "CHANNEL_create" a "CHANNEL_recv":

  Int  CHANNEL_create (Channel_t *ch, Uns   mode,   Uns  channelNr); 
  Void CHANNEL_destroy(Channel_t  ch); 
  Int  CHANNEL_send   (Channel_t  ch, char *buffer, Int  nbytes);
  Int  CHANNEL_recv   (Channel_t  ch, char *buffer, Int *nbytes, Int           timeout);
  Int  CHANNEL_asend  (Channel_t  ch, char *buffer, Int  nbytes, CHANNEL_Rqst *rqst);
  Int  CHANNEL_arecv  (Channel_t  ch, char *buffer, Int *nbytes, CHANNEL_Rqst *rqst);
  Int  CHANNEL_test   (CHANNEL_Rqst *chRqst, Bool *flag); 
  Int  CHANNEL_wait   (CHANNEL_Rqst *chRqst); 
  Int  CHANNEL_waitall(Int count, CHANNEL_Rqst *chRqst); 
  Int  CHANNEL_waitany(Int count, CHANNEL_Rqst *chRqst, Int *index);


Las secciones de texto del kernel de IDSP (kernel + oper + group + edma_isr) ocupa 32384 bytes (menos de 32 Kb):
  
  init     0220h
  oper     2F80h
  kernel   2940h
  group    2020h
  edma_isr 0380h
  _______________
  total    7E80h = 32384 bytes




. 03/Noviembre/03. Idsp_58
El paquete STREAM_CODEC se ha mejorado en lo referente a la detección del offset introducido por los Codecs TLC320AD535 del DSK. Anteriormente presentaba una condición de carrera que obligaba un operador tipo SPEAKER a determinar el offset antes que un operador tipo MICROPHONE. Esto obligaba a GROUP_create a disponer adecuadamente el arranque de estos operadores, algo inaceptable.

Se ha introuducido el paquete AUDIO_STREAM, construido sobre STREAM_CODEC. Oculta a los operadores el problema del offset de los TLC320AD535. Su interfaz es:

  Int  AUDIO_STREAM_create    (AUDIO_STREAM_Stream_t *stream, Int   ioMode,   Int   bufferDim, Int windowDim, Int codec);
  Int  AUDIO_STREAM_destroy   (AUDIO_STREAM_Stream_t  stream);
  Int  AUDIO_STREAM_get       (AUDIO_STREAM_Stream_t  stream, Int **window_0, Int **window_1);
  Int  AUDIO_STREAM_put       (AUDIO_STREAM_Stream_t  stream, Int  *window_0, Int  *window_1);
  Int  AUDIO_STREAM_getEnergy (AUDIO_STREAM_Stream_t  stream, Int  *energy);

El núcleo se ha dividido en dos: idsp_kernel y idsp_gcrpc. El paquete GCRPC está construido sobre elkernel, pero, de momento, el kernel hace upcalls a GCRPC. El núcleo prácticamente se ha reducido a una primitiva universal de envío y otra de recepción:

  Int  COMM_send      (COMM_Comm_t srcComm, Int syncMode, char *buffer, Int count, ADDR_Addr_t dstAddr, Int tag, RQST_Rqst_t *rqst);
  Int  COMM_recv      (COMM_Comm_t dstComm, Int syncMode, char *buffer, Int count, ADDR_Addr_t srcAddr, Int tag, RQST_Rqst_t *rqst, COMM_Status *status, Int timeout);

Sobre esta base, GCRPC ofrece una interfaz de comunicación de grupo (GC) -de momento con primitivas punto a punto únicamente- y una interfaz diferenciada de llamada a procedimiento remoto (RPC). La interfaz de GCRPC es:

  Int  GC_send      (char        *buffer, Int count, Int dst, Int tag);  
  Int  GC_recv      (char        *buffer, Int count, Int src, Int tag, Int cix, COMM_Status *status, Int timeout); 
  Int  GC_asend     (char        *buffer, Int count, Int dst, Int tag,          COMM_Rqst_t *rqst);  
  Int  GC_arecv     (char        *buffer, Int count, Int src, Int tag, Int cix, COMM_Rqst_t *rqst); 
  Int  GC_wait      (                     COMM_Rqst_t  rqst,                    COMM_Status *status);  
  Int  GC_waitall   (Int          count,  COMM_Rqst_t *rqst,                    COMM_Status *status[]);
  Int  GC_waitany   (Int          count,  COMM_Rqst_t *rqst, Int *index,        COMM_Status *status);
  Int  GC_test      (                     COMM_Rqst_t  rqst, Int *flag,         COMM_Status *status);  

  Int  RPC_trans    (char        *buffer, Int count, Int service);
  Int  RPC_send     (char        *buffer, Int count, Int dst);
  Int  RPC_recv     (char        *buffer, Int count, Int src);




. 16/Diciembre/03. Idsp_59
La interfaz de canal se completa. Observemos que el parámetro "time-out" se desplaza desde "CHANNEL_create" a "CHANNEL_recv":

La primitiva

  Int  CHANNEL_recv   (Channel_t  ch, char *buffer, Int *nbytes, Int timeout);

pasa a

  Int  CHANNEL_recv   (Channel_t  ch, char *buffer, Int  nbytes, Int timeout);

y ahora devuelve el número de octetos recibidos.

Las primitivas de espera por E/S asíncronas devuelven ahora un estado. De este modo se puede determinar el emisor/receptor, el número de octetos recibidos/enviados, etc:

  Int  CHANNEL_wait   (           CHANNEL_Rqst *chRqst,             CHANNEL_Status *chStatus); 
  Int  CHANNEL_waitall(Int count, CHANNEL_Rqst *chRqst,             CHANNEL_Status *chStatus); 
  Int  CHANNEL_waitany(Int count, CHANNEL_Rqst *chRqst, Int *index, CHANNEL_Status *chStatus);




. 15/enero/2004. Idsp_60
Los paquetes GC y RPC se han segregado de COMM.

RQST mejora. Ahora se almacena un timeout absoluto en el objeto struct RQST_Rqst. 

OPER_unready vuelve ahora con error si fue deshabilitado. Esta nueva semática se parece al hecho de que una llamada al sistema UNIX vuelva con error si el proceso invocante recibió una señal. OPER_unready devuelve un código de error que discrimina si el thread invocante cumplió su plazo de espera o bien fue deshabilitado.

Se ha fijado un error detectado por JUan Antonio Rico en COMM_mailboxGet.

COMM_waitany devuelve ahora un código de error que diferencia si vuelve debido a la expliración del plazo o debido a que el thread invocante ha sido deshabilitado. En el caso de expiración de plazo, el nuevo campo Timeout de COMM_Status se pone a TRUE.

COMM_send mejora con la introducción de plazo relativo en la entrega y se hace simétrico a COMM_recv:
  Int  COMM_send      (COMM_Comm_t srcComm, Int syncMode, char *buffer, Int count, ADDR_Addr_t dstAddr, Int tag, COMM_Rqst_t *rqst,                      Uns timeout);
  Int  COMM_recv      (COMM_Comm_t dstComm, Int syncMode, char *buffer, Int count, ADDR_Addr_t srcAddr, Int tag, COMM_Rqst_t *rqst, COMM_Status *status, Uns timeout);

Se introduce el envío con plazos en la interfaz de canal:
  Int  GC_send   (char *buffer, Int count, Int dst, Int tag,                  Uns timeout);  
  Int  GC_asend  (char *buffer, Int count, Int dst, Int tag, GC_Rqst_t *rqst, Uns timeout);  

Se introduce el envío con plazos en la interfaz de canal y se introducen asimismo las primitivas de comunicación asíncrona:
 Int  CHANNEL_recv   (Channel_t  ch, char *buffer, Int  nbytes,                     Uns timeout);
 Int  CHANNEL_arecv  (Channel_t  ch, char *buffer, Int  nbytes, CHANNEL_Rqst *rqst, Uns timeout);

CHANNEL_Status se define como sigue: Cuando CHANNEL_recv o CHANNEL_arecv vuelven por timeout, el campo TimeoutCounter contiene el número de ramas del fan-out que han vuelto con error.



. 20/enero/2004. Idsp_61

IDSP_61 no ofrece adiciones, pero realiza un cambio sustancial, que es una recomposición de los módulos que componen IDSP.
Se redefine el kernel, hasta ahora el paquete COMM, aumentando este a costa de algunos de los servicios que antes prestaba el servidor de operadore "idsp_oper". El paquete COMM es ahora un paquete hijo del nuevo kernel, denominado ahora KNL. Desaparece el paquete "idsp_other", que se integra en KNL. El paquete "oper_server" pasa a denominarse OPR. El paquete "group_server" pasa a denominarse GRP. El paquete idsp_channel" pasa a denominarse "CHN. En suma, IDSP tiene ahora una estructura más clara que lo hace más modular y por tanto más fácil de comprender y manejar.




. 22/enero/2004. Idsp_62

El parametro size de "create" desaparece. La primitiva "exit" desaparece. 

. 26/enero/2004.
El registro de operadores desaparece del núcleo y se integra en OPR. Así, "create" cambia de nuevo, ya que necesita la información que antes obtenía del registro:

 Int create(Oper_t *oper, ADDR_Oper_t addr, Fxn bodyFxn, Int stackSize, char *param, Int paramSize);

"linked_operators" ha desaparecido. Los operadores de usuario hacen ahora #include <cfg.h> y #include <opr.h>

. 27/enero/2004.
La primitiva enroll se ha simplificado. Ya no invoca a GRP para obtener un gix, sino que lo genera como el número aleatorio devuelto por CLK_gethtime. Una consecuencia inmediata es que, por primera vez, KNL no depende ni de OPR ni de GRP: Ya no hay "upcalls" del núcleo a los servidores.

. 28/enero/2004.
He simplificado el módulo OPR_REG. He tomado medidas de los tamaños de los módulos de IDSP en bytes:

  GRP 20a0h  8352
  OPR 1180h  4480
  KNL 17A0h  6048
  COM 20E0h  6048
  LNK 1420h  5152
  ISR 03C0h  0960
  GC  0580h  1408
  RPC 0420h  1056
  CHN 0CE0h  3296

Según estas cifras, IDSP ocupa 39 Kb, una cifra bastante buena si consideramos que DSP/BIOS tiene 25 Kb:
  
  KNL + COM                                          = 14464
  KNL + COM + OPR + GRP                              = 27296
  KNL + COM + OPR + GRP + ISR                        = 28256
  KNL + COM + OPR + GRP + ISR + LNK                  = 33408
  KNL + COM + OPR + GRP + ISR + LNK + GC + RPC       = 35872
  KNL + COM + OPR + GRP + ISR + LNK + GC + RPC + CHN = 39168 (< 39 Kb)


. 04/febrero/2004.
Nuevo sistema de excepciones para el sistema. Ahora hay dos formas de compilar el núcleo, modo depuración (definiendo la constante _DEBUG_ en el fichero "xpn.h"), y el modo ordinario (sin definir dicha constante). Los nuevos tamaños, en modo depuración, son:

  GRP 23E0h
  OPR 12E0h
  KNL 1820h  
  COM 2200h  
  LNK 14C0h  
  ISR 03E0h  
  GC  05C0h  
  RPC 0460h  
  CHN 0F60h  

Los nuevos tamaños, en modo no debug son:

  GRP 1E60h
  OPR 10C0h
  KNL 1680h  
  COM 1DA0h  
  LNK 1380h  
  ISR 0340h  
  GC  0480h  
  RPC 0360h  
  CHN 0B20h  que hacen un total de 8D00h, es decir, 36096 (< 36 kb)

Ahora sólo KNL y COMM invocan panic.



. 06/febrero/2004. Idsp_64
He descubierto y corregido un bug en COMM_waitany

. 07/febrero/2004. 
La interfaz de GC_ aumenta con el rango comodín GC_RANK_ANY. Esta modificación ha sido motivada porque era imposible para un canal (módulo CHN_) recibir de dos o más canales diferentes. CHN_arecv y CHN_arecv utilizan ahora GC_RANK_ANY. 


. 08/febrero/2004. 
La interfaz de ADDR_ cambia y se simplifica a 

  extern Bool ADDR_match(const ADDR_t addr,  const ADDR_t dst);

Esto hace más rápido el mapeo de direcciones cuando llega un paquete. Afecta a RQST_lookUp y a COMM_mailboxGet. También hace más pequeño el módulo COM, que pasa de 1DA0h a COM 1D00h. 


. 18/febrero/2004. 
Desaparece el concepto de máquina raíz como la máquina que alberga al servidor de grupos. De ahora en adelante, todas las máquinas disponene de un servidor de grupos igual que disponen de un servidor de operadores. El paquete GRP_REG desaparece. El nuevo tamaño de GRP en modo no debug es

  GRP 1C40h

. 22/febrero/2004. 
Se ha fijado un bug en CHN_superSend.


. 26/febrero/2004. 
Se ha introducido una nueva primitiva en el kernel, setPriority:

#define setPriority(prio) TSK_setpri(TSK_self(), prio)


. 27/febrero/2004. 
Se ha introducido una nueva primitiva en el kernel, stat:

  Void stat(Opr_t oper, Stat *stat)

DEvuelve la estadística de actividad de un operador.


. 28/febrero/2004. 

La versión idsp_64 introduce un avance significativo. El servidor de grupos ejecuta ahora en TODAS las máquinas. A partir de ahora, cuando un operador desea crear un grupo, hace un RPC con el servidor GRP local. Se elimina, pues, el único punto de fallo que suponía un servidor de grupos único para todo el sistema residente en la máquina raíz. Desaparece el concepto de máquina raíz. 

Los grupos consumen recursos en la máquina que los gestiona. La terminación inesperada de un operador del grupo malogra el grupo completo y deja basura en el servidor de grupos. Para resolver el problema se ha introducido un recolector de basura en el servicio de grupos, un thread periódico del servidor GRP local (en "GRP_svr.c") que se engarga de solicitar la actividad de los operadores de los grupos gestionados localmente. Cuando la actividad de un operador es nula o el operador simplemente no responde a las soliciitudes de actividad, se destruye el grupo, liberando los recursos que consume. Una consecuencia es que se ha eliminado la primitiva GRP_leave.

Los nuevos tamaños, en modo no debug son:

  GRP 1CA0h
  OPR 1160h
  KNL 1740h  
  COM 1C40h  
  LNK 1380h  
  ISR 0340h  
  GC  04C0h  
  RPC 0400h  
  CHN 0B40h  que hacen un total de 8c40h, es decir, 35904 (35 kb)



. 03/marzo/2004. Idsp_65

He simplificado el paquete COMM_GMAP basado en que ahora sólo hay un comunicador por operador. Ahora el paso de mensajes debe ir ligeramente más rápido. El nuevo tamaño de COM es más reducido:

  COM 1940h  

. 09/marzo/2004.

CHN ya no necesita el apoyo de KNL. La ventaja es que KNL y CHN se desacoplan completamente. El efecto es que, si bien KNL se ha simplificado, CHN se ha complicado bastante. Este aumento de complejidad ha hecho necesario el paquete hijo CHN_ENV. Los nuevos tamaños son:

  KNL 1560h  
  CHN 1080h  



. 14/marzo/2004. Idsp_66

Esta versión incorpora el código generado automáticamente por la "Herramienta de configuración" desarrollada por Juan Antonio Rico Gallego. El directorio "gen" contiene este código. Cada sesión de la herramineta de configuración creará un subdirectorio en "gen". En adelante aparecerá el directorio "gen" en la distribución al mismo nivel que "system" o "drv". 

El código automático confía que IDSP_66 desacopla el sistema de los operadores. Ya no hay un OPR_REG inicializado con los atributos de 5 o de 23 operadores preestablecidos, sino un OPR_REG vacío. La creación de un operador mediante OPR_create conlleva el registro previo del mismo. Para llevar a cabo este registro se ha dispuesto la nueva primitiva OPR_register.

OPR ha disminuido de tamaño

  OPR 1060h


. 30/marzo/2004.
La nueva gestión gestión de los grupos introducida el 28 de febrero hace innecesario el cabo cliente, que desaparece.
Los canales han sido desacoplados del núcleo. El tamaño de CHN aumenta. El tamaño del núcleo se reduce. Los nuevos tamaños son: 

   GRP 0C20h
   OPR 1000h
   KNL 1560h  
   COM 18E0h
   LNK 1380h 
 + ISR 0340h  
 ------------
       6120h =   24864 bytes
 
   GC  04C0h  
 + RPC 0400h  
 ------------
        8C0h =    2240 bytes
 
   CHN 1720h =    5920 bytes
                ------------
 TOTAL       =   33024 bytes (32 Kb)



. 28/junio/2004. Idsp_67

La nueva primitiva 

  Int  CHN_grpSet    (Int  gix, Int *inCh, Int *outCh);

configura un grupo recién creado "gix" con su grafo de canales.

CHN es ahora una biblioteca completamente desacoplada del núcleo: Ni KNL_ ni OPR_ conocen su existencia. 


. 13/julio/2004. Idsp_68
Se introduce el nivel de red NET (por Jesús María Álvarez Llorente) para soportar la ejecución de IDSP en multicomputadores Sundance.

La primitiva de KNL wait se redefine:

  #define WAIT_ANY    0 
  #define WAIT_ALL    1 
  Int wait(Opr_t oper, Int mode, COMM_Rqst_t *rqst, Int count, Int *index);

COMM se ha hecho más robusto porque COMM_send y COMM_recv se han construido sobre COMM_waitany. COMM_waitall se ha implementado sobre el nuevo wait de KNL en modo WAIT_ALL.

La interfaz de CHN cambia. Desaparecen CHN_waitany y CHN_waitall por que su implementación es demasiado costosa. CHN se restringe a:

  Void CHN_open      (CHN_t *ch, Uns   mode,   Uns  chnNr); 
  Int  CHN_send      (CHN_t  ch, char *buffer, Int  nbytes, Uns timeout);
  Int  CHN_asend     (CHN_t  ch, char *buffer, Int  nbytes, Uns timeout);
  Int  CHN_recv      (CHN_t  ch, char *buffer, Int  nbytes, Uns timeout);
  Int  CHN_arecv     (CHN_t  ch, char *buffer, Int  nbytes, Uns timeout);
  Void CHN_test      (CHN_t  ch, Bool *flag) ;
  Int  CHN_wait      (CHN_t  ch, CHN_Status *chStatus); 
  Void CHN_terminate ();

La implementación de CHN_wait ha mejorado sustancialmente haciendo uso del nuevo COMM_waitall. El recolector de basura es ahora más sencillo. Se da un tiempo de vida a cada entorno, 60 puntos. Cada segundo se resta un punto. Una comunicación (un CHN_send, por ejemplo, restaura los 60 puntos)



. 26/julio/2004. Idsp_69
Se ha trabajado en el nivel de red NET para que este acepte paquetes de tamaño arbitrario. Hasta ahora los paquetes eran de pequeño tamaño, el exigido por las aplicaciones de audio como AIRE. Nos proponemos utilizar idsp en aplicaciones de imágenes donde se esperan paquetes de tamaño mucho mayor. En el envío, el nivel de red NET fragmenta los paquetes en otros más pequeños que entrega al nivel de enlace LNK_MCBSP. En la recepción no hay código adicional. Los mensajes que transportan los fragmentos se entregan al operador destino. La nueva implementación de RQST_satisfy acumula los fragmentos hasta que la petición de recepción se satisface completamente. Sólo entonces se entrega a la aplicación. Este diseño no introduce copias adicionales y, por lo tanto, no tiene impacto alguno en el rendimiento. Una restricción importante: Si se envía un mensaje de, digamos 12842 octetos, el operador destinatario debe llevar a cabo una recepción de exatamente 12842 octetos.

En segundo lugar, se ha trabajado en el nivel de enlace sobre la exploración del buffer cíclico. Se ha eliminado el objeto "Packet_Descriptor" que despues se transformaba en un objeto RQST_msgDptr. En idsp_69 se extrae directamente este último. No hay copias de cabeceras, lo que conlleva una ganancia importante de prestaciones. 



. 29/julio/2004. Idsp_70
Se ha actuado en NET_send para que rute paquetes partidos en el buffer cíclico, no presente en idsp_69. No hay sobrecarga adicional.


. 21/agosto/2004.
Se ha actuado sobre LNK_MCBSP: Se ha desacoplado de KNL y NET, configurándose como un módulo que puede utilizarse con independencia de IDSP. En consecuencia un programa ordinario DSP/BIOS puede ahora utilizarlo. Las versiones anteriores asumían que LNK_MCBSP transportaba datos con el formato RQST_msgDptr. Ahora LNK_MCBSP se limita a transportar marcos sin entrar a examinar su contenido.  El formato del marco es:
 
   ¡-----------¡---------¡--------¡       
   ¦ data SIZE ¦  DATA   ¦  TAIL  ¦       
   '-----------'---------'--------'   

LNK_MCBSP entrega el campo de datos al nivel superior invocando una función de éste (upcall), que previamente registra en LNK_MCBSP. Para llevar a cabo el registro, LNK_MCBSP ha habilitado una nueva primitiva a su interface:

  Void LNK_MCBSP_register (Int(*upcall)(Vbuffer *vector, Int vcnt));


La implementación del nivel NET se ha modificado para adaptarse a la nueva interfaz de LNK_MCBSP. En particular, NET_deliver es la función que NET registra en LNK_MCBSP como upcall. Es NET_deliver quien construye el objeto RQST_msgDptr, que bien entrega al núcleo o bien encamina de nuevo. 



. 25/agosto/2004. Idsp_71
Se ha actuado sobre NET para desacoplarlo del núcleo de la misma forma que en la versión anterior se desacopló LNK_MCBSP. La nueva interfaz es:

  Void NET_init     (Void);
  Void NET_register (Int (*upcall)(Vbuffer *vector, Int vcnt));
  Int  NET_send     (Header_t header, char *buffer, Int count);

Como se aprecia, NET no necesita ver el tipo RQST_MsgDptr. No realiza, por tanto, la conversión del paquete entrante a este formato, sino que deja esta tarea al núcleo destinatario. El routing se beneficia, ya que no es preciso construir un objeto RQST_MsgDptr por cada paquete. Por el contrario, se retransmite tal cual al nivel de enlace como una simple copia vía XDMA. 

Se ha actuado sobre la implementación de COMM, concretamente sobre "comm.c", para soportar la nueva interfaz de NET. A diferencia de las últimas versiones, ahora COMM no ve NET en una red de un único procesador sin modo loopback. El fichero net.c, por tanto, pierde el soporte de la configuración de red NET_NO_NETWORK.

KNL se ha modificado. Ahora es COMM_init quien invoca NET_init. El módulo ADDR se ha simplificado. Desaparece la implementación "addr.c" de KNL, que se ha sustituido por macros en el nuevo "addr.h". Este se ha dispuesto fuera del núcleo, al mismo nivel que "cfg.h", para que pueda ser visto por KNL y NET.



. 26/agosto/2004. Idsp_72
Se ha actuado sobre sobre "comm.c" y "comm_rqst" de COMM. La función interna COMM_expectMsg pasa a denominarse COMM_expect. COMM_expect y COMM_deliver se han modificado. Ahora aglutinan las sentencias de sincronización en la recepción (COMM_expect) y el envío de  (COMM_deliver) de mensajes. Estas sentencias estaban antes dispersas entre estas funciones y la función RQST_satisfied. Esta última pasa a denominarse RQST_feed. El resultado es un COMM mucho más limpio y de más calidad. 


. 27/agosto/2004.
Se han realizado modificaciones menores en GC, KNL y COMM.


. 31/agosto/2004.
Se ha actuado en RPC. RPC_send se ha implementado directamente sobre NET, eliminando COMM_send. RPC aumenta con una nueva primitiva:

  Void RPC_init (Int *cpuId);
 
RPC_init devuelve el identificador de la máquina en la que ejecuta para adjuntarla a los mensajes que envía el nuevo RPC_send.  

Se ha eliminado la constante THIS_MACHINE. Esta era realmente un seudónimo de una variable global de NET, NET_this_machine, que "veía" el núcleo. Para eliminar esta dependencia oculta se ha modificado la primitiva NET_init:

   Void NET_init(Int *cpuId);

Ahora NET_init devuelve el identificador de la máquina al nivel invocante COMM, un método más serio que la variable global. En configuraciones de red de una sóla máquina, el identificador es 0. COMM reeleva a KNL el identificador de la máquina a través de la nueva COMM_init:

  Void COMM_init(Int *cpuId);
  
KNL proporciona la máquina a RPC en RPC_init. Para ello se ha actuado sobre KNL, que aumenta con una primitiva más:

  Int  getCpuId(Void);

GetCpuId permite a los procesos de usuario conocer la máquina en la que residen.


.2/septiembre/2004.
Se ha actuado en "chn_svr.c" de CHN. La nueva función interna CHN_init arranca el servidor de canales. Se invoca en CHN_grpSet. De este modo el servidor sólo arranca cuando existe en la máquina un grupo interesado en utilizar CHN. Este cambio simplifica la función "AppMain", de "appmain.c", que ahora no necesita ocuparse de CHN.




.12/septiembre/2004. Idsp_73
Se ha actuado en ADDR, en "addr.h": Desaparecen las definiciones de constantes OPR_GIX y CHN_GIX, que van a parar a sus respectivos paquetes.
El objeto Addr pasa de 

struct Addr {
  Int Mix;  
  Int Gix;
  Int Oix;     
};
typedef struct Addr Addr, *Addr_t;

a

struct Addr {
  Int Gix;
  Int Oix;     
};
typedef struct Addr Addr, *Addr_t;

Esto conlleva una mayor transparencia a la ubicación de todo IDSP. En particular, GC no ve máquina alguna. Tampoco CHN. RPC sigue involucrado con máquinas, que se utiliza para operar directamente sobre objetos de máquinas concretas. Por esta razón se ha mantenido el concepto de "dirección de un operador" como:

struct OprAddr {
  Int  Mix;  
  Addr Addr;  
};
typedef struct OprAddr OprAddr, *OprAddr_t;




.13/septiembre/2004.
Desaparece el paquete KNL_CELL, cuya única finalidad era guardar una única copia por máquina de la tabla [rango, máquina]. Ahora la tabla es un campo del descriptor del operador. La solución es redundante y requier más memoria, pero reduce código. Futuros desarrollos determinarán si el cambio es bueno.

Actuación en CHN. Se ha descubierto un bug en CHN_open, de "chn_svr.c". La determinación del canal de entrada del operador destinatario de un canal de salida se hacía de forma incorrecta, ya que no se hacía la comunicación con la máquina que albergaba el operador destinatario. 

Actuación en NET. Perdemos la independencia del núcleo en NET, ya que NET_send invoca getMachine. Este es un cambio transitorio, para probar lo antes posible Idsp_73 en los multicomputadores Sundance.  




.19/septiembre/2004. Idsp_74
El cambio en ADDR introdujo un bug en RQST_feed, que asignaba una máquina remitente errónea a los mensajes entrantes. El bug no se detectaba en un DSK (red de una máquina única). Se ha corregido.

El mismo cambio en ADDR arruinó las funciones de KNL setclient y getclient. Estas dos funciones se han modificado.

La versión idsp_73 tenía otro error. Los mensajes enviados a la misma máquina también se enviaban a NET. Este error no se detectaba en un DSK. Se ha corregido. 

Recuperamos en NET la independencia del núcleo perdida en idsp_73.




.19/septiembre/2004. Idsp_75
Los paquetes KNL, COMM, OPR, GRP, GC y RPC se han fundido en un único paquete KNL. Ocupa 20288 octetos, menos de 20k.
La biblioteca CHN ocupa 6752 octetos, menos de 7k.

.22/septiembre/2004.
Actuación sobre GRP. GRP_create crea ahora un grupo vacío. El nuevo GRP_join añade un nuevo componente al grupo, especificando la máquina en que debe arrancar. Si la máquina es DEFAULT_MACHINE, el sistema elige una. Ejemplo:

  GRP_create (&groupId, GRP_GRAPH);
  for(i = 0; i < ADD_MAX; i++)
    GRP_join(groupId, oper[i], mchn[i], i, param[i], paramSize[i]);

.24/septiembre/2004.
Actuación sobre CHN con objeto de aumentar en calidad. Los nombres de los ficheros de CHN cambian. La estructura de interfaces es ahora estrictamente la que sigue:

    ¡-----------------------¡  
    |                       | 
    |          CHN          | 
    |                       | 
    |          ¡------------| 
    |          |            | 
    |          |  CHN_GLB   | 
    |          |            | 
    |----------'------------| 
    |                       | 
    |        CHN_ENV        | 
    |                       | 
    '-----------------------' 


.26/octubre/2004.
IDSP funciona correctamente en el multiplicador Sundance. 


.1/noviembre/2004.
Cambios en init(), que absorbe la creación de una tarea en main.c. 


.8/noviembre/2004.
Desaparece el proyecto idsp_xx.pjt. En el directorio principal de la distribución aparece ahora el subdirectorio "example". Contiene tres directorios, uno por entorno hardware. Cada uno contine el fichero .cdb correspondiente y main.c, este último igual para todos. 

El directorio app pasa a denominarse "operator". Los directorios "include", "lib" y "Debug" desaparecen. 



.19/febrero/2005. Idsp_751
Segregación de la gestión de operadores y la carga de CPU. El paquete OPR es ahora

  Void OPR_init       (Void);
  Int  OPR_create     (OPR_t *oper, Addr_t addr, Int machine, Int name, char *param, Int paramSize);
  Int  OPR_destroy    (OPR_t  oper);
  Int  OPR_start      (OPR_t  oper); 
  Int  OPR_kill       (OPR_t  oper);
  Int  OPR_setMachines(OPR_t  oper, Int  *mixTable, Int grpSize);
  Int  OPR_getStat    (OPR_t  oper, Stat *statistics);

Notese que el parámetro "addr" sólo aparece en OPR_create. 

Aparece el paquete LOAD, servido por su propio skeleton, antes integrado en OPR. Es:

  Int  LOAD_get     (Int  machine, Int *load);
  Int  LOAD_getLBusy(Int *machine);




.21/marzo/2005. Idsp_752
Aparece el paquete REG, registro de operadores. Antes de compilar una imagen IDSP, el usuario edita los siguientes ficheros:

  reg_names.h    Identificadores de operadores. Es obligatorio que figuren aquellos que intervienen en la imagen
  reg_proto.h    Prototipos de dichos identificadores.
  reg_linked.h   Establece los operadores que intervienen en la construcción de la imagen


Los operadores enlazados con IDSP se especifican en la tabla "lnkTable" de "reg_linked.h". El usuario debe editar esta tabla.

.21/marzo/2005.
El kernel deja de utilizar MEM_alloc y MEM_free. Usa ahora KER_alloc y KER_free para crear y destruir los objetos thread KER_Obj_t. 
Por tanto, idsp no utiliza memoria dinámica.

.23/marzo/2005.
El paquete COMM ha desaparecido, absorbido por KER. La interfaz de KER es ahora más limpia. Los upcalls desde COMM son ahora funciones privadas de KER. 

.24/marzo/2005.
La primitiva send adquiere el nuevo parámetro "mchn". Es la máquina a la que va dirigida el mensaje. Antes la determinación de la máquina destino, una invocación a "getMachine", estaba oculta en RQST_set. Ahora la determinación de la máquina destino lo hace exclusivamente el nivel GC, un lugar mucho más conveniente. El diseño del núcleo KER, por tanto, mejora, reconociendo que se limita a enviar octetos a una máquina y proceso concreto. La transparencia a la ubicación de los operadores de un grupo se implementa en GC.

Una consecuencia de este cambio es que RPC_send se ha podido implementar ahora sobre send, mientras que antes lo hacía sobre NET_send, puenteando KER. 

La función "main" se extrae del proyecto principal y se oculta incroporada al núcleo KER. El código de usuario va en la función AppMain del fichero "appmain.c".

.27/marzo/2005.
Desaparecen del paquete RQST las dependencias de <cfg.h> y <clk.h>. Ahora es directamente portable.


.01/abril/2005.
Las comunicaciones locales se han acelerado. La primitiva de KER_send se apoya en NET_send. Hasta ahora, NET no veía el paquete RQST y, por lo tanto no podía ver el tipo RQST_Msg_t. La nueva interfaz de NET es:

  #include <rqst.h>
  Void NET_init     (Int        *cpuId);
  Void NET_install  (Int       (*upcall)(RQST_Msg_t msg, Bool local));
  Int  NET_send     (RQST_Msg_t  msg, Bool *sync_mode);
  Int  NET_broadcast(RQST_Msg_t  msg);

Ahora KER_send pasa a NET_send directamente el descriptor del mensaje RQST_Msg_t, en lugar de -desmontar- el descriptor para pasar un header y la dirección de los datos, un par de punteros, al fin y al cabo. Como NET_send recibe ahora el mensaje, puede invocar directamente el upcall de KER "deliver"

  Int deliver(RQST_Msg_t msg, Bool localSrc), 

en lugar de invocar el upcall de KER "rawDeliver", que -montaba- de nuevo el descriptor del mensaje. Montar un mensaje lo venía haciendo la función de KER "buildMsg".

En cuanto a los mensajes que llegan de NET desde una máquina remota, estos llegan descritos por dos punteros al buffer cíclico. Ahora se puede montar el mensaje como el objeto RQST_Msg_t en NET, de modo que "buildMsg" se extrae de KER y se incorpora a NET. Nótese que el upcall "rawDeliver" 

  Int rawDeliver(Vbuffer *vector, Int vcnt, Bool local);

ha desaparecido. 


EDMA_ISR_init sale de la inicialización de KER, de "trueInit", y entra en la inicialización de LNK_MCBSP, en "LNK_MCBSP_init". En consecuencia, KER ya no depende de EDMA_ISR.


.03/abril/2005.
Actuación en GMAP para evitar posibles condiciones de carrera. Era posible que dos threads creados casi al mismo tiempo en una máquina obtuvieran el mismo gix. También era posible entregar el mensaje para un hilo reción destruido a un hilo creado inmediatamente después.
Evitar estas condicones de carrera conlleva una comunicación más lenta. El módulo en conjunto ha mejorado.



.05/abril/2005. Idsp_753
Signal y wait son dosfunciones internas de KER. Su interfaz se simplifica porque cae el primer parámetro, que se puede extraer del segundo, la petición implicada:

  Void  signal (          Rqst_t  rqst);
  Int   wait   (Int mode, Rqst_t *rqst, Int count, Int *index);

La interfaz de GRP cambia. GRP_create devuelve ahora un objeto Group_t en lugar de un gix.



.07/abril/2005.
El cambio en GRP create supone un problema en la implementación de CHN. Resolverlo implica añadir una nueva primitiva a GRP:

  Int  GRP_getGix     (Group_t  obj); 


.07/abril/2005.
Se ha eliminado el parámetro "arrived" de la función "mbxGet" y ahora esta devuelve TRUE o false en función de si ha llegado un mensaje que casa.  Esto permite la optimización de la función interna de KER "expect" porque se elimina la variable local "arrived".


.08/abril/2005.
Desaparece "expect", que no es natural, y se integra en "recv".


.09/abril/2005.
Para ganar rapidez, "recv" se implementa ahora sobre "wait" en vez de sobre "waitany". 


.10/abril/2005.
Para ganar rapidez, "recv" en modo síncrono no recurre a RQST_alloc y RQST_free, sino que utliza la estructura RqstSyncRecv, un nuevo campo del descriptor de thread. Ello conlleva la pequeña complicación en "deliver" de comprobar también RqstSyncRecv del thread destinatario.
Parte del paquete RQST se ha reimplementado como macros para ganar en rapidez.

.11/abril/2005.
"send" en modo síncrono no recurre a RQST_alloc y RQST_free. Como consecuencia, el uso de este último se ha estructurado definitivamente, siendo invocado en waitany, waitall y test.


.21/abril/2005. Idsp_754
Se ha mejorado MBX_get


.26/abril/2005. Idsp_760
------------------------
Se crea "msg.h". NET depende ahora de msg.h, no de KER/rqst.h.


.29/abril/2005.
Hasta ahora las peticiones de envío-recepción "RQST_Obj" las proporcionaba KER asignándolas del pool de peticones del descriptor de thread.
En IDSP_760 las peticiones las proporciona la aplicación. Desaparecen por tanto los pool como tales, que pasan a ser un registro de peticiones pendientes. Desaparecen por tanto las primitivas de pool RQST_initPool, RQST_alloc y RQST_free, y aparecen RQST_register y RQST_unregister.

Aparece la primitiva MSG_set (implementada en "msg.h"). Su uso simplifica ahora la primitiva RQST_set. Desparece la primitiva RQST_getMsg.


.05/mayo/2005
Para ganar rapidez, la función de KER "signal" se ha reimplementado de modo más eficiente. Desaparecen las funciones internas "getWaitRqst" y "setWaitRqst". Esta última almacenaba el vector de peticiones "rqst" en que se bloqueaba un thread, bien como waitany o como waitall. Ahora estas peticiones simplemente se etiquetan poniendo a cierto su campo "Suspend" y su índice en el vector en el campo "SuspendIdx"


.24/mayo/2005
El objeto
  struct OPR_Obj {
    Thr_t  Thr;
    Int    Machine;
  };

pasa a ser
  struct OPR_Obj {
    Addr   Addr;
    Int    Machine;
  };
El puntero remoto no es una buena solución porque puede haber sido "reciclado" cuando se usa. La dirección única es un método mucho más seguro de registrar un thread remoto, si bien es menos eficiente. Impone la búsqueda del thread tomando como clave su dirección. Ello ha origina laintroducción de una nueva llamada al sistema en KER, "getThr":

  Thr_t getThr        (Addr_t);


Desaparecen los recogedores de basura de GRP y CHN. Desaparece por tanto la llamada al sistema de KER stat, así como OPR_stat y su servicio.


.26/mayo/2005. 
La invocación de la función "deliver" de KER por parte de la tarea LNK_MCBSP_scan del paquete LNK_MCBSP introduce una condición de carrera. Para eliminarla se impide que LNK_MCBSP_scan interrumpa a KER. Para ello:
 1. El núcleo KER se convertido en un monitor monolítico: Se eleva la tarea que entra en el mismo a la prioridad más alta TSK_MAXPRI 
    y volviendo esta a su prioridad original cuando sale.
 2. Haciendo que LNK_MCBSP_scan ejecute a la misma prioridad que KER.




.30/mayo/2005. Idsp_761
-----------------------
La invocación de la primitiva de KER destroy por parte del servidor OPR ocasionaba una condición de carrera con la invocación de destroy por parte de un un thread que termina su ejecución. Veamos. Supongamos que el servidor recibe el mensaje OPR_DESTROY para eliminar la tarea T. La función "Opr_destroy" sirve la petición, invocando la sentencia

  destroy(getThr(&addr));

Durante la ejecución de la misma, el servidor OPR podía ser expulsado precisamente por la tarea T tras el retorno de getThr. La tarea T podía invocar destroy(self()), eliminando su descriptor y terminando su ejecución. Nótese que cuando OPR retoma el procesador el parámetro de destroy no es válido. Incluso puede haber sido reciclado para una nueva tarea Q. La consecuencia unala segunda destrucción de T, que ocasiona un fallo de DSP/BIOS, o incluso algo peor, la destrucción de Q. 

La resolución del problema pasa por la ejecución atómica de 

  destroy(getThr(&addr));

algo que se consigue ejecutando el servidor OPR con la máxima prioridad.


.01/junio/2005
Se ha acometido el problema de la ejecución atómica de 

  destroy(getThr(&addr));

que acarreaba la ejecución de los servidores OPR y LOAD con la máxima prioridad.

La función getThr se hace interna al kernel y se la despoja del cambio de prioridades. Para ello ha sido necesario cambiar la interfaz de procesos a:

  Int   init          (Void);
  Int   enroll        (Void);
  Void  leave         (Void); 
  Int   create        (Addr_t addr, Fxn bodyFxn, Int stackSize, Int prio, char *param, Int paramSize);
  void  destroy       (Addr_t addr); 
  Int   start         (Addr_t addr); 
  void  kill          (Addr_t addr);

Como se aprecia, desaparece el objeto Thr_t, así como el método "self", ahora interno. Esto lleva a una interfaz de gestión de procesos más elegante pero más lenta, ya que destroy, start y kill deben buscar el objeto Thr_t a partir de su parámetro Addr_t. El beneficio último de estos cambios es que los servidores OPR y LOAD ejecutan ahora con la mínima prioridad y no interfieren en la velocidad de las comunicaciones de procesos ya arrancados.


.03/junio/2005
La función getThr desaparece, ya que su funcion ya está prevista en el paquete GMAP, concretamente en la función GMAP_lookup, que además está escrita para ser rápida. Con ello el problema de la mayor lentitud de la gestión de procesos queda prácticamente solucionado. 




.6/junio/2005. Idsp_762
-----------------------
El paquete REG desaparace absorbido por OPR como OPR_REG. Esta disposición es más natural, ya que los métodos de REG sólo eran utilizados por el servidor de OPR. Los efectos de este cambio son:

  1. El directorio "include" de "auto" desaparece. 
  2. OPR aumenta su interfaz con dos nuevos ficheros .h:
       a) Dispone los nombres de los operadores en "opr_names" 
       b) Dispone los operadores enlazados en "opr_linked"
     

Los servicios OPR_destroy,OPR_kill y OPR_setMachines  ya no devuelven réplica. Cambian por lo tanto opr_stub, opr_skeleton y opr.h. La semántica es más natural y el servicio es más rápido. De forma similar, el servicio al mensaje CHN_ENV_DESTROY tampoco devuelve réplica.




.6/junio/2005. Idsp_763
-----------------------
Los campos del objeto Addr de ADDR cambian de nombre.

El paquete KER se ha convertido en un verdadero monitor, con la puerta de entrada denominada KER_ENTER() y la de salidad denominada KER_EXIT(). El monitor se ha implementado como una región crítica guardado por el mutex "kerMutex". Un thread que se bloquea dentro del monitor invoca KER_EXIT inmediadamente antes de la suspensión. De igual forma, la primera acción de la reanudación es invocar KER_ENTER.


Cuando un paquete viene de otra máquina a través de la red, NET irrumpe en el núcleo invocando la función interna de KER "deliver" sin someterse al protocolo de KER_ENTER y KER_EXIT. Se puede decir, pues, que dos tareas:

  1: La tarea de usuario que invoca la primitiva de KER "send" 
  2. La tarea del buffer cíclico, que invoca la función de NET "NET_deliver" 

compiten por el recurso "deliver". Así pues, este debe ejecutar en región crítica y, para ello hace uso del mutex de guarda "dlvMutex".




.10/junio/2005. Idsp_764
------------------------
Se estudia identificar los bloques de instrucciones de las primitivas de KER que necesitan realmente ejecutar en región crítica (dentro del monitor) y reducir estos al mínimo posible:

  KER_ENTER();
  {
    /* Bloque */
  }
  KER_EXIT();

De esta forma se reduce al mínimo el bloqueo que las tareas de menor prioridad imponen a las de prioridad mayor.

La reflexión ha puesto de manifiesto que, para el caso de "send", no es necesario el registro de peticiones asíncronas pendientes. El campo "SendPendReg" del descriptor de thread, pues, desaparece.


En un DSK en modo loopback el envío de un mensaje suponía enviar al MCBSP vía EDMA cuatro bloques distintos:

  1. Tamaño de los datos
  2. Cabecera
  3. Datos
  4. Cola

Juan Antonio Rico Gallego ha implementado un nivel de enlace sobre Sundance Digital Bus (SDB) basado en paquetes de tamaño fijo (hasta 1 Kbyte). Esto permite transferir via EDMA sólamente

  1. Tamaño de los datos
  2. Cabecera, que incluye el tamaño de los datos que van en el paquete

Este diseño mejora la velocidad de comunicación en un 25% aproximadamente. 


Otra ventaja del paquete de tamaño fijo S es que el buffer cíclico es un múltiplo de S, por lo que los mensajes no quedan partidos en ambos extremos del buffer cíclico. Los dos campos del mensaje Data_Size[0] y Data_Size[1], e igualmente Data_Begin[0] y Data_Begin[1], no son necesarios, pero se siguen utilizando por compatibilidad con el DSK en modo loopback. Se introduce sin embargo en en el mensaje el nuevo campo de la cabecera "Size". El nuevo campo afecta a las funciones del NET "NET_send" y "buildMsg". 




.17/junio/2005. Idsp_765
------------------------
Se ha dispuesto el paquete GMAP como un monitor utilizando un semáforo. Lo mismo ocurre con las primitivas "alloc" y "free" de KER. El resultado es que toda la interfaz de gestión de procesos no necesita de bloques  

  KER_ENTER();
  {
    /* Bloque */
  }
  KER_EXIT();

Estos han quedado reducidos a tres, en las funciones:

  1. deliver
  2. receive
  3. wait




.19/junio/2005. Idsp_766
------------------------
A) Ficheros fuente afectados: ker.c, rqst.c y rqst.h
La primitiva RQST_set ha mejorado. Antes tomaba como parámetro un mensaje de tipo "Msg" que era preciso construir previamente vía MSG_set. Ahora este parámetro desaparece sustituido por otros cuatro que ya contenía el antiguo parámetro: la dirección fuente/destino del mensaje, su etiqueta, buffer y tamaño. La desaparición del parámetro ahorra una llamada a "MSG_set" en la primitiva "recv" de KER, lo cual aclara su implementación y mejora su rendimiento. "MSG_set" sólo se utiliza ahora en la primitiva de KER "send", que es lo natural -construir un mensaje antes de entregarlo a "NET_send" para enviarlo-. 

La mejora de la primitiva RQST_set está basada en la modificación del objeto RQST_Obj, cuyo campo "Msg" se sustituye por el campo "Status", mucho más pequeño.     




.21/junio/2005. Idsp_767
------------------------
A) Ficheros fuente afectados: ker.c, rqst.c y rqst.h
La primitiva RQST_set cambia ligeramente, admitiendo más parámetros y pasa a llamarse RQST_open. Aparece RQST_close. El módulo RQST adopta ahora para la petición un paradigma open-find-feed-close.  Estos cambios en la interfaz RQST hacen a KER más fácil de comprender.





.29/junio/2005. Idsp_768
------------------------
A) Ficheros fuente afectados: ker.c.
La primitiva recv se apoya en en "waitall" en vez de en "wait" 
El código de comprobación de peticiones cerradas de "waitall" y "waitany" pasa a "wait" para ejecutar en región crítica.


cambia ligeramente, admitiendo más parámetros y pasa a llamarse RQST_open. Aparece RQST_close. El módulo RQST adopta ahora para la petición un paradigma open-find-feed-close.  Estos cambios en la interfaz RQST hacen a KER más fácil de comprender.





.3/julio/2005. Idsp_769
------------------------
A) Ficheros fuente afectados: ker.c y ker.h
El código de comunicación de KER que debe ejecutar en región crítica se segrega de KER como en el nuevo paquete MON (de monitor). La interfaz de MON es:

  Int  MON_init    (void); 
  Int  MON_open    (RQST_Obj *rqst, Addr_t srcAddr, char *buffer, Int count, Int sync, Int tag, Thr_t me, Uns timeout);
  Int  MON_wait    (RQST_Obj *rqst, Int count, Int mode, Int *index);
  Int  MON_deliver (Msg_t msg, Bool local); 

 



.3/julio/2005. Idsp_770
------------------------
A) Ficheros fuente afectados: rpc.c 
Se simplifica la implementación de RPC_recv porque ya no reanuda "recv" tras una excepción RPC_E_DISABLED.

 



.18/octubre/2005. Idsp_771
--------------------------
...


.20/octubre/2005
----------------
Ficheros fuente afectados: grp.h y grp.c, opr.h y opr.c

- opr.c y opr.h
  Como GRP no sabe nada ahora de la máquinas en las que corren sus operadores, debe preguntárselo a OPR. OPR aumenta pues con la primitiva
  
    Int  OPR_getMachine  (OPR_t  oper);  
 
- grp.c y grp.h
  La primitiva  GRP_getMachines cambia a:

    Int  GRP_getMachines(Group_t grp, int *grpSize, int *machine, int mchnMax);


.21/octubre/2005
----------------
- grp.c y grp.h
  El cambio en GRP_getMachines no es bueno para su uso en CHN. La consecuencia es doble:
  1. Aparece la primitiva 

    Void GRP_getSize(Group_t grp, int *grpSize);

  2. La primitiva  GRP_getMachines cambia a:

    Void GRP_getMachines(Group_t grp, int *machine);


.25/octubre/2006
----------------
Se ha detectado el problema de los mensajes huérfanos. Un mensaje huérfano es aquel que, cuando se recibe, se detecta que su emisor ha muerto.
El problema se maniifestón por primera vez en el sevidor de canales, en el bucle de servicio CHN_service_loop de "chn_gbl_skeleton.c". Como el mensaje no contiene los datos, sino un puntero a los mismos en el "espacio" del emisor. Si se mata al emisor y otro ocupa su espacio, el mensaje apunta a basura y la acción que provocaba era simplemente errónea.

El problema se ha terminado de resolver hoy. Sólo ha sido necesario introducir unas pocas líneas en MON_open de "mon.c". Prácticamente no afecta a las prestaciones de "recv". Cuando se detecta un mensaje huérfano, se anota este extremo y MON_open continúa extrayendo mensajes hasta completar la petición de recepción en curso. Sólo entonces MON_open retorna, devuelviendo la nueva excepción MON_E_HORPHAN que propaga a recv y este a su vez a RPC_recv, etc. 

El servidor CHN_service_loop recibía antes las peticiones de la forma siguiente:

  while(1) { 
    if(0 > (xpn = RPC_recv((char *)&rqst_msg, sizeof(Chn_Msg), RPC_ANY)))      goto exception;
    ...

mientras que ahora utiliza la excepción MON_E_HORPHAN para identificar e ignorar una petición huérfana

  while(1) { 
    if(0 > (xpn = RPC_recv((char *)&rqst_msg, sizeof(Chn_Msg), RPC_ANY))) 
      if(excpn == -RPC_E_HORPHAN) continue; else goto exception;
    ...

Como se aprecia, el servidor se ha hecho robusto a peticiones huérfanas con muy poco código y máxima eficiancia.





.29/octubre/2006. Idsp_800
--------------------------
La tabla de los operadores enlazados se saca del núcleo para que no haya que recompilarlo cada vez que se cambia esta tabla. Además la recompilación de núcleo requiere que el usuario disponga de su código fuente. Ahora la tabla la proporciona el usuario. Se ha modificado para que no ahora no utilice tipo REG_Entry, que se convierte ahora en un tipo privado de OPR. además, se elimina el paquete hijo OPR_REG, absorbido por OPR. Tras estos cambios, en adelante el usuario:

1. Debe proporcionar una tabla de entradas int de la forma

  static int oprTable[] = {
                     /* Name                     Body Function            Stack size 
                        ----                     -------------            ---------- */
                        #if SOURCE_LINKED
                        SOURCE_OPERATOR,         (Fxn)Data_Source,        1024,
                        #endif

                        ...

                        #if ADDER_LINKED
                        ADDER_OPERATOR,          (Fxn)Adder,              1024,
                        #endif
						/* El ultimo debe ser este */
                        NULL_OPERATOR,           NULL,                    NULL,
                     };


2. Debe registar en OPR dicha tabla mediante la nueva primitiva de OPR 

   Int  OPR_register (int *table); 






.06/noviembre/2006. Idsp_801
----------------------------
La implementación de GRP cambia. Reaparece el servidor de grupos. La interfaz es ahora

  void GRP_init       (void);
  void GRP_newGix     (int *gix);   
  int  GRP_getSize    (int  gix, int *size);
  int  GRP_getMachines(int  gix, int *machine, int size); 

  int  GRP_create     (int  gix, int *machine, int size);  
  int  GRP_join       (int  gix, int rank, int name, int prio, void *param, int paramSize);
  int  GRP_start      (int  gix);
  int  GRP_kill       (int  gix);
  void GRP_destroy    (int  gix);

La nueva implementación permite que la interfaz sea visible no sólo al hilo creador (que invoca GRP_create) sino a todos los hilos del grupo.



.13/noviembre/2006
------------------
Desaparece OPR (incluyendo el servidor de operadores) y sus funciones son asumidas por GRP. No obstante, el nombre OPR se reutiliza para el registro de operadores:

  int  OPR_register    (int   *table); 
  int  OPR_getStackSize(int   *stackSize, int name);
  int  OPR_getBody     (Fxn   *function,  int name);


La implementación de GRP se simplifica. Observese que desaparece GRP_kill, absorbido por GRP_destroy:

  void GRP_init       (void);
  void GRP_newGix     (int *gix);   
  int  GRP_getSize    (int  gix, int *size);
  int  GRP_getMachine (int  gix, int rank, int *machine); 

  int  GRP_create     (int  gix, int *machine, int size);  
  int  GRP_join       (int  gix, int rank, int name, int prio, void *param, int paramSize);
  int  GRP_start      (int  gix);
  void GRP_destroy    (int  gix);

No obstante, GRP absorbe a CG. La interfaz anterior aumenta con:

  Int  GRP_send   (char *buffer, Int count, Int dst, Int tag,                   Uns timeout);  
  Int  GRP_asend  (char *buffer, Int count, Int dst, Int tag, GRP_Rqst *rqst,   Uns timeout);  
  Int  GRP_bcast  (char *buffer, Int count, Int root); 
  Int  GRP_recv   (char *buffer, Int count, Int src, Int tag, GRP_Status *status, Uns timeout); 
  Int  GRP_arecv  (char *buffer, Int count, Int src, Int tag, GRP_Rqst *rqst,   Uns timeout); 

  #define GRP_test    test
  #define GRP_waitany waitany
  #define GRP_waitall waitall
  #define GRP_wait(a, b) GRP_waitany(1, &a, NULL, b)



.17/noviembre/2006. Idsp_802
----------------------------
Desaparece GMAP, cuya función es asumida por GRP. El tamaño del código se ha reducido, pero no mucho.

Desaparecen de cfg.h las constantes de configuración del núcleo, entre ellas RNK_MAX. La desaparición de RNK_MAX ha conllevado una reducción grande del área de datos.

La interfaz de KNL se ha vuelto más limpia

  int   init          (void);
  int   enroll        (void);
  void  leave         (void); 
  int   create        (Thr_t *thr, Addr_t addr, Fxn bodyFxn, int stackSize, int prio, char *param, int paramSize);
  void  destroy       (Thr_t  thr); 
  int   start         (Thr_t  thr); 
  void  kill          (Thr_t  thr);

  int   send          (Addr_t dst, int mchn, char *buf, int cnt, int sync, int tag, RQST_Obj *rqst, Uns timeout, Addr_t src);
  int   recv          (Addr_t src,           char *buf, int cnt, int sync, int tag, RQST_Obj *rqst, RQST_Status *status, Uns timeout);
  int   waitany       (int count, RQST_Obj *rqst, int *index, RQST_Status *status);
  int   waitall       (int count, RQST_Obj *rqst,             RQST_Status *status);
  void  test          (           RQST_Obj *rqst, int *flag,  RQST_Status *status);  

  Addr  getAddr       (void);
  int   getCpuId      (void);

  #define      setPriority(prio) TSK_setpri(TSK_self(), prio)
  extern void  setClient     (int  mix, Addr  addr);
  extern void  getClient     (int *mix, Addr *addr);
  extern void  panic         (char *msg); 






.20/noviembre/2006. Idsp_803
----------------------------
Se retira GRP_getThr de grp.h, ya que sólo la usa mon.c, que ahora incorpora la declaración

 extern int  GRP_getThr(int  gix, int rank, Thr_t *thr); 


El campo State del descriptor de hilo "Thr" de "mon.h" pasa a ser el mapa de bits [ENABLED, READY, ALLOCATED]. Desaparecen los campos "Enabled"  y "Allocated". Esta solución es más elegante y ahorra espacio.


El número máximo de descriptores de thread no es fijo. El usuario puede establecerlo en tiempo de enlace mediante el fichero "ker_thread_max.h" 
Lo mismo ocurre con las tablas de GRP, cuyo tamaño se establece en el fichero "grp_grp_max.h"





.21/noviembre/2006. Idsp_804
----------------------------
La primitiva de KNL 

  int   send          (Addr_t dst, int mchn, char *buf, int cnt, int sync, int tag, RQST_Obj *rqst, Uns timeout, Addr_t src);

pierde el argumento "mchn". Conseguimos transparencia a la ubicación. Cuando se invoca un RPC con un servicio en una máquina concreta, el argumento tag debe ser RPC_TAG. En tal caso el argumento "dst" contiene en el campo "Group" el grupo del servidor. Como el servidor es un grupo único, el campo "Rank" es necesariamente cero. Aprovechamos este hecho para disponer en el campo "Rank" la máquina destinataria.


Desaparecen de la interfaz de KNL las primitivas setClient y getClient, ya que sólo las usa RPC. Con todo, la interfaz de KNL queda como sigue

  int   init          (void);
  int   enroll        (void);
  void  leave         (void); 
  int   create        (Thr_t *thr, Addr_t addr, Fxn bodyFxn, int stackSize, int prio, char *param, int paramSize);
  void  destroy       (Thr_t  thr); 
  int   start         (Thr_t  thr); 
  void  kill          (Thr_t  thr);

  int   send          (Addr_t dst, char *buf, int cnt, int sync, int tag, RQST_Obj *rqst, Uns timeout, Addr_t src);
  int   recv          (Addr_t src, char *buf, int cnt, int sync, int tag, RQST_Obj *rqst, RQST_Status *status, Uns timeout);
  int   waitany       (int count, RQST_Obj *rqst, int *index, RQST_Status *status);
  int   waitall       (int count, RQST_Obj *rqst,             RQST_Status *status);
  void  test          (           RQST_Obj *rqst, int *flag,  RQST_Status *status);  

  Addr  getAddr       (void);
  int   getCpuId      (void);

  void  setPriority   (int prio);
  void  panic         (char *msg); 


.22/noviembre/2005
------------------
Se hacen mejoras en las directivas #include de los diversos ficheros de CHN.

Vuelve a aparecer GC, que se segrega de GRP.

Las primitivas de MON toman nombres más apropiados. MON_deliver se denomina ahora MON_send y MON_open se denomina ahora MON_recv.

  int  MON_init (void); 
  int  MON_recv (RQST_Obj *rqst, Addr_t srcAddr, char *buffer, int count, int sync, int tag, Thr_t me, Uns timeout);
  int  MON_send (Msg_t msg, Bool localSrc);
  int  MON_wait (RQST_Obj *rqst, int count, int mode, int *index);




.23/noviembre/2005. Idsp_805
----------------------------
Se partió "ker.c" en dos ficheros distintos. Uno para la gestión de threads y otro para la comunicación. Apareció un bug en las pruebas, de modo que esta versión no se libera. Se pasa directamente a la idsp_806 que comienza desde idsp_804.




.24/noviembre/2005. Idsp_806
----------------------------
Se introdude en GRP la primitiva GRP_setThr. La implementación de GRP_join la utiliza.

GRP_setThr sustituye a GRP_join cuando se crean threads del sistema, como el daemon de GRP o LOAD, o de biblioteca, como el daemon de CHN. Una consecuencia beneficiosa es que CHN_OPERATOR desaparece de "opr_names.h". Tambien lo hacen GRP_OPERATOR, LOAD_OPERATOR y NULL_OPERATOR.

Aparece en GRP primitiva GRP_daemon. Realmente es una función de biblioteca. Reune las instrucciones de GRP para crear servidores. Hemos hecho uso de ella para crear los bucles de servicio (daemons) de LOAD, GRP y CHN. Sus ventajas son fundamentalmente dos, estructura y ahorro de código.





.24/noviembre/2005. Idsp_807
----------------------------
KER no ve ahora OPR. Es AppMain quien invoca OPR_register, tal y como ocurría antes. Este es el diseño correcto.

Se ha partido "ker.c" en dos ficheros distintos. Uno para la gestión de threads, "ker.c" y otro para la comunicación, "com". En la inicialización, la función "trueInit" de KER invoca el nuevo método de COM "COM_init". COM_init iniciliza la red. La clave en esta segregación de funciones ha sido introducir un nuevo fichero, thr.h, que contiene la definición del objeto THr_t.



 



.12/diciembre/2005. Idsp_808
----------------------------
Azequia se inicializa ahora con AZQ_init. Es necesario hacer #include <azq.h> para invocarla. Por otra parte, AppMain se denomina ahora AZQ_main.

El paquete OPR se convierte en un paquete hijo de GRP. El usuario no lo ve. AZQ_init se invoca con un parámetro que es la tabla de operadores proporcionada por el usuario.


En "msg.h" cambia la definición de MSG_set. Desaparece el parametro "sense" ya que sólo se utiliza MSG_set para el envío, en COMM_send. Desaparecen de msh.h las constantes MSG_SEND y MSG_RECV.

El paquete KER se denomina ahora THR. Thr.h pasa a llamarse thr_dpt.h, ya que define fundamentalmente eldescriptor de thread. Así, ker.c pasa a llamarse thr.c. ker.h pasa a llamarse thr.h. 

GRP_destroy pasa a llamarse GRP_kill. Este nombre responde a su función realmente.

GRP_daemon pasa a denominarse GRP_single.

La función main de DSP/BIOS abandona THR y pasa a formar parte de AUTO.



.20/diciembre/2005. 
---------------------------- 
Se han mejorado las primitivassend y recv de COM. La nueva interfaz sustituye el parámetro "sync" por "mode". Mode es un mapa con los siguientes bits.

#define MODE_ASYNC     0x00000001    /* For synchronous/asynchronous mode */
#define MODE_RPC       0x00000010    /* For RPC/GC mode                   */
#define MODE_SVR       0x00000100    /* For client/server user            */

Por defecto, send y recv son síncronos a menos que "mode" tome el valor MODE_ASYNC. Los otros dos bits sólo afectan a send y son utilizados por COM para implementar el nivel RPC. Al usuario sólo le afecta MODE_ASYNC. RPC activa el bit MODE_RPC siempre que utiliza send y recv. Además, un servidor RPC activa el bit MODE_SVR. Por otra parte, el parámetro de send "srcAddr" desaparece. Las primitivas quedan:

       int   send       (Addr_t dst, char   *buf, 
                                     int     cnt, 
                                     int     mode, 
                                     int     tag, 
                                     Rqst   *rqst,                 
                                     Uns     timeout);
       int   recv       (Addr_t src, char   *buf, 
                                     int     cnt, 
                                     int     mode, 
                                     int     tag, 
                                     Rqst   *rqst, 
                                     Uns     timeout, 
                                     Status *status);


El nivel RPC se ha rediseñado para utilizar puertos globales. Ahora un cliente no envía a una dirección [gix, máquina], sino a un puerto global  [port] que identifica el servicio. Como interesan no sólo los servicios en general, sino los servicios que están en una máquina dada, se ha definido un puerto (en rpc.h) como 

  struct Port {
    int Port;
    int Mchn;
  };
  typedef struct Port Port, *Port_t;

Cuando no importa la máquina que presta el servicio, en el campo Mchn se pone a DFLT_MCHN. 

Las nuevas primitivas RPC especifican un parámetro Port_t en lugar del antiguo RPC_Addr_t: 

  int  RPC_trans  (char *buffer, Int count, const Port_t addr);
  int  RPC_send   (char *buffer, Int count, const Port_t addr);
  int  RPC_recv   (char *buffer, Int count, const Port_t addr);

Ahora los servidores deben publicar sus servicios. Un servidor RPC "S" con dirección "[G, 0]" que ejecuta en una máquina M y que presta el servicio "P" registra este en una tabla de entradas [port, gix]. Su entrada será concretamente [P, G]. Por supuesto, todo cliente de P necesita en último término de "G" para enviar a "S" sus peticiones. Afortunadamente, es el núcleo cliente quien resuelve el problema. Para ello la implementación de RPC_send interroga a la máquina M utilizando un nuevo servicio, el así denominado PORTMAP, un paquete hijo de RPC. PORTMAP tiene la siguiente interfaz:

  int  PORTMAP_init      ();
  int  PORTMAP_get       (int svc, int *gix, int mchn);
  int  PORTMAP_register  (int svc);
  void PORTMAP_unregister(int svc);

El servidor invoca PORTMAP_register en su rutina de inicialización para anunciar que presta el servicio "P". RPC_send utiliza PORTMAP_get para conocerla dirección del servidor. Nótese que el usuario no ne PORTMAP, sólo proporciona puertos.

Los servidores del sistema GRP y LOAD han sido reimplementados con este sistema. Antes todo servidor de grupos tenía la dirección [GRP_GIX, 0] en todas las máquinas. Ahora tiene un Gix aleatorio y diferente en cada máquina. Todos los servidores GRP seaddceden mediante el puerto [GRP_PORT, mchn]. El servidor de canales, por supuesto, también utiliza este esquema. 






.22/diciembre/2005. Isdp_809
---------------------------- 
En su estado actual, muchos ficheros fuente contienen la sentencia #include <std.h>, un fichero de DSP/BIOS. El problema estriba en que algunos de ellos
  a) No hacen uso de la interfaz de funciones exportada por DSP/BIOS (como las llamadas TSK_, SEM_, etc), 
  b) Sí hacen uso de los tipos simples exportados por DSP/BIOS, fundamentalmente Uns,Bool, Ptr y Fxn. 

El objetivo ha sido eliminar la innecesaria dependencia de DSP/BIOS que sufren de tales ficheros, es decir, eliminar la sentencia #include <std.h>. 

La solución ha sido la sustitución de estos tipos por otros propios de Azequia. Se introduce, pues, el fichero "azq_types.h" que define estos tipos. El tipo Ptr se ha eliminado de Azequia, sustituido por su definición original "void *". El fichero es:

  #ifndef _AZQ_TYPES_H_
  #define _AZQ_TYPES_H_ 

  typedef int (*fxn)();		
  typedef int bool;		
  typedef unsigned uns;		

  #ifndef NULL
  #define NULL 0
  #endif

  #ifndef TRUE
  #define FALSE ((Bool)0)
  #define TRUE  ((Bool)1)
  #endif

  #endif


En la nueva versión: 
  1. Un fichero que importaba tipos de std.h y sí usaba la interfaz funcional de DSP/BIOS añade la sentencia #include <azq_types.h> 
  2. Un fichero que importaba tipos de std.h y no usaba la interfaz funcional de DSP/BIOS cambia la sentencia #include <std.h> 
     por #include <azq_types.h>

Esta medida reduce la dependencia de DSP/BIOS y la confina a los ficheros de tipo 1. De hecho, en estos se ha dispuesto el comentario:
 "/* Use of DSP/BIOS */". La nueva versión Isdp_809 es más fácilmente portable.







.25/diciembre/2005. Isdp_810
---------------------------- 
El fichero "msg.h" desaparece absorbido por net.h. 

RQST_open cambia ligeramente su interfaz introduciendo un parámetro mapa de bits "mode" que agrupa si la petición es de envío o recepción y si es síncrona o asíncrona. La nueva versión de la primitiva RQST_open es:

  int RQST_open (Rqst_t rqst, int mode, Addr_t addr, int tag, char *buf, int count, void *me, RQST_PendReg_t reg, uns timeout);  

La primitiva de RQST RQST_feed ha sido reimplementada.


.28/diciembre/2005.
-------------------

  Un nuevo valor de retorno para recv
  -----------------------------------
Hasta ahora idsp imponía que el buffer de recepción de "recv" tuviese de tamaño los mismos octetos que los que le iban a ser enviados por una invocación de "send". Los cambios en RPC que han aparecido en esta versión, y que luego comentaremos, no podían llevarse a cabo con esta restricción. La solución ha sido disponer un buffer muy grande para todas las situaciones y que "RPC_recv" devuelva el número de octetos que ha llegado. Conocer este número pasa por que la primitiva COM "recv" devuelva los octetos que han llegado al buffer de recepción. ¿Cómo se ha hecho esto? Vamos a explicarlo. Recibir supone establecer una petición de recepción que la primitiva RQST "RQST_feed" va llenando con los sucesivos fragmentos de mensaje entrantes. Un fragmento responde al tipo Msg:

  #define CHUNK_MAX  2                      
  struct Msg {
    int         DstMchn;
    Addr        Dst;                   /* Do            [1]  */
    int         SrcMchn;               /* not           [2]  */
    Addr        Src;                   /* change        [3]  */
    int         Tag;                   /* this order!!! [4]  */ 
    void       *Thr;
    int         WholeSize;             /* Whole message size */
    int         FgmtSize;              /* Fragment size */
    void       *Rqst;                  /* Corresponding send request in local rendez-vous */
    int         Chunks_Nr;             /* Either 1 or 2 */
    char       *Data_Begin[CHUNK_MAX]; 
    int         Data_Size [CHUNK_MAX];
  };
  typedef struct Msg Msg, *Msg_t;

El campo "Size" se ha renombrado como "FgmtSize", más significativo, y se ha introducido el campo "Wholesize", que contiene el tamaño del mensaje oriiginal producido por "send" antes de la fragmentación que lleva a cabo el nivel NET. Cada fragmento entrante provoca la invocación de "RQST_feed", cuyo nuevo prototipo es:

  int RQST_feed (Rqst_t rqst, const Msg_t msg, int *acum, bool *completed);

El nuevo parámetro de salida "acum" informa sobre el número total de octetos contribuidos por los fragmentos entregados a la petición "rqst". Cuando "acum" coincide con "Wholesize" es señal de que el último fragmento acaba de llegar. El mensaje original ha sido recibido. Nótese el contraste con la solución antigua, que detectaba el final del mensaje cuando se llenaba el buffer de recepción. Esta solución es sencilla y casi no presenta penalización en el tiempo de ejecución. La primitiva "recv" se implementa sobre de COM "waitall". "Waitall" no devuelve como hasta ahora COM_E_OK, sino el campo "Count" de su parámetro de salida "Status" correspondiente a la primera petición de su parámetro vector de peticiones. Este valor de retorno se propaga como valor de retorno de "recv". Sobre esta nueva "recv" se construye el nuevo RPC. 


 El nuevo RPC
 ------------
Un servidor RPC desconoce la petición que le va a llegar y el tamaño de sus parámetros y, por lo tanto, le es imposible  conocer el tamaño del buffer que "RPC_recv" debe suministrar. Hasta la fecha, el obstáculo se ha venido solucionando de una forma "ad hoc" poco eficiente y nada elegante, ya que el procedimeinto de servicio debía comportarse como cliente, haciendo invocaciones RCP al cliente para solicitarle sus argumentos. Exponer a los usuarios finales a esa facilidad RPC tan primitiva era cuando menos poco serio. 

La nueva interfaz RPC sigue casi fielmente la del sistema operativo distribuido Amoeba:

  int  RPC_send      (Hdr_t hdr, void *buff,  int cnt);
  int  RPC_recv      (Hdr_t hdr, void *buff,  int cnt);
  int  RPC_trans     (Hdr_t hdr, void *buff1, int cnt1, void *buff2, int cnt2);
  int  RPC_register  (int port);
  void RPC_unregister(int port);

Los clientes IDSP sólo invocan ahora RPC_trans, mientras que los servidores sólo invocan RPC_recv y RPC_send. 

Empezemos por el cliente, con RPC_trans. Un servicio tiene un identificador denominado "Puerto". Lo elige el programador del servicio y debe ser conocido por los clientes. Un servicio puede ser prestado por una o más máquinas, de modo que el puerto se representa como una estructura "Port", de dos campos:

  struct Port {
    int Port;
    int Mchn;
  };
  typedef struct Port Port, *Port_t;

Port.Port es el identificador propiamente dicho mientras que Port.Mchn es la máquina en la que ejecuta el servidor. El cliente puede elegir la que desee si es conocedor de que allí ejecuta un servidor. Si desconoce quién presta el servicio, que es la situación más frecuente, especifica DFLT_MCHN. RPC se encarga de buscar una máquina que preste el servicio. 

El parámetro "hdr" es un encabezado de 128 octetos dividido en dos campos, Port y Store:

  struct Hdr {
    Port Port;
    int  Store[32 - (sizeof(Port)/sizeof(int))];
  };
  typedef struct Hdr Hdr, *Hdr_t;

El campo "Port" es el puerto donde un cliente envía las peticiones. Una petición consta de un encabezado "Hdr" y un buffer en su caso. El campo "Store" no tiene estructura. Cliente y servidor se ponen de acuerdo en sus contenidos. Actualmente se utiliza para almacenar el denominado "mensaje del servicio", un objeto que contiene el identificador del método invocado y los parámetros del mismo más bien pequeños como enteros o estructuras ligeras. El campo "buff1" lo utiliza el cliente para enviar parámetros grandes como vectores, cadenas de octetos, etc. El campo "cnt1" esel tamaño de "buff1". "Store" también se utiliza como repositorio de los parámatros pequeños de salida. De manera parecida, en el campo "buff2" el cliente recaba parámetros de salida grandes. El campo "cnt2" es el tamaño de "buff2". RPC_trans devuelve el número de octetos acumulados en "buff2".

RPC_recv y RPC_send sólo los utilizan los servidores. Un servidor utiliza RPC_recv de la siguiente forma. Llena el campo Port del parámetro "Hdr" con su número de puerto. El resto de "Hdr" no se toca. Dispone un buffer "buff" de tamaño "cnt" que se llenará totalmente o en parte con la llegada de la petición. La invocación de RPC_recv bloquea el servidor hasta la llegada de una petición. El encabezado entrante contiene el mensaje del servicio y sobreescribe el dispuesto en RPC_recv. El buffer entrante se almacena en el buffer dispuesto por RPC_recv. El tamaño del buffer entrante es devuelto por RPC_recv. El servidor utiliza RPC_send para enviar la réplica. Dispone en el campo "Store" de un objeto "Hdr" (puede ser el utilizado en RPC_recv) el mensaje del servicio y en "buff" un posible parámetro de salida de tamaño "cnt".

Antes de iniciar el bucle de servicio, un servidor RPC debe dar a conocer que es él el thread que presta el servicio (que escucha en el puerto asociado al servicio). Para ello invoca RPC_register, al que pasa el puerto como parámetro. Cuando el servidor termina, indica esta circunstancia invocando RPC_unregister.


  Los mensajes vacíos
  -------------------
Hasta ahora "send" detectaba el intento de construir y enviar un mensaje vacío, es decir, sin datos. Su reacción era ignorar el intento en silencio. Tanto RPC_send como RPC_trans del nuevo RPC envían dos buffers. El primero es un encabezado de 128 octetos y el segundo un buffer posiblemente vacío. Igualmente tanto RPC_recv como RPC_trans reciben estos dos buffers. Como secomprende, si el segundobuffer no se envía por ser nulo, la recepción permanecerá bloqueada para siempre. Esto ha obligado a no ignorar el envío de un buffer nulo, sino enviar un mensaje vacío. Sorprendentemente no ha sido necesario cambiar nada en COM ni MON. La petición de recepción se completa inmediatamente con la llegada del mensaje vacío. 



.29/diciembre/2005. Isdp_811
----------------------------

  El cliente muerto
  -----------------
En esta versión se prueba el que un thread del grupo y no su creador opere sobre el grupo. La operación ha sido GRP_kill. La prueba ha sido un éxito como cabía esperar. Se ha producido una incidencia inédita hasta ahora que consiste en que el cliente muere después de que el servidor tome la petición, pero antes de que entregue la réplica. Esta situación no estaba adecuadamente tratada y se ha reparado. Parte de lareparación ha sido elcambio de nombre de la excepción de XXX_E_DEADSRC a XXX_E_DEADPART, que recoge los dos casos de que tanto el thread destinatario como el remitente no está disponible. ¿Por qué se ha manifestado este problema ahora? Porque un miembro del grupo invoca un RPC_trans al servidor de grupos. Mientas el cliente está bloqueado esperando la réplica el servidor le señala. Si el cliente tiene mayor prioridad que el servidor, la situación más común, expulsa al servidor y vuelve de RPC_trans con error RPC_E_DISABLED. Este error está planteado para inducir al suicidio inmediato. Entonces el servidor retoma el procesador y envía una réplica con RPC_send a un thread que no existe. RPC_send vuelve con la excepción RPC_E_DEADPART, que ahora el servidor captura e ignora.


  Las interfaces ocultas
  ----------------------
Los ficheros include han sufrido una fuerte transformación. El problema era:

  1. Presentar al usuario un conjunto de ficheros .h que describiesen las interfaces de THR, COM, GC, RPC y GRP sin dar detalles sobre 
     su implementación 
  2. Que la implemntación correspondiente no sufriese una pérdida de rendimiento.

La solución se basa en:

  1. Partir cada fichero de interfaz xxx.h en dos, uno con el mismo nombre xxx.h que sólo contiene la especificación de los métodos y constantes que se exportan, y xxx_hddn.h, que contiene detalles de implementación que no interesan al usuario y que, a la postre, dan pistas sobre la implementación en su conjunto. A modo de ejemplo, el descriptor de thread se ha ocultado en thr_hddn.h. 
  2. Desplazar los ficheros xxx_hddn.h del directorio "spec" al directorio "impl", junto con los ficheros .c

El resultado es que el directorio "spec" contiene la interfaz y sólo la interfaz de idsp.





5. Problemas conocidos
En la versión 19 decíamos: Hay una derivación de memoria dinámica muy difícil de detectar.
No está claro en absolutio que idsp padezca una deriva de memoria. La asignación de memoria dinámica de DSP/BIOS no es perfecta. No abre huecos más grandes cuando se libera memoria. Por demostrar.