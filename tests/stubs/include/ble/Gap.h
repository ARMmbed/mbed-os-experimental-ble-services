/* mbed Microcontroller Library
 * Copyright (c) 2006-2020 ARM Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BLE_GAP_GAP_H
#define BLE_GAP_GAP_H

#include "ble/common/CallChainOfFunctionPointersWithContext.h"

#include "ble/common/BLETypes.h"
#include "ble/gap/Events.h"
#include "ble/gap/Types.h"

namespace ble {

#if !defined(DOXYGEN_ONLY)
namespace impl {
class Gap;
}
#endif // !defined(DOXYGEN_ONLY)

/**
 * @addtogroup ble
 * @{
 * @addtogroup gap
 * @{
 */

/**
 * Define device discovery, connection and link management procedures.
 *
 * - Device discovery: A device can advertise to nearby peers its existence,
 * identity and capabilities. Similarly, a device can scan its environment to
 * find advertising peers. The information acquired during the scan helps to
 * identify peers and understand their use. A scanner may acquire more information
 * about an advertising peer by sending a scan request. If the peer accepts scan
 * requests, it may reply with additional information about its state.
 *
 * - Connection: A bluetooth device can establish a connection to a connectable
 * advertising peer. Once the connection is established, both devices can
 * communicate using the GATT protocol. The GATT protocol allows connected
 * devices to expose a set of states that the other peer can discover, read and write.
 *
 * - Link Management: Connected devices may drop the connection and may adjust
 * connection parameters according to the power envelop needed for their
 * application.
 *
 * @par Accessing gap
 *
 * Instance of a Gap class for a given BLE device should be accessed using
 * BLE::gap(). The reference returned remains valid until the BLE instance
 * shut down (see BLE::shutdown()).
 *
 * @code
 * // assuming ble_device has been initialized
 * BLE& ble_device;
 *
 * ble::Gap& gap = ble_device.gap();
 * @endcode
 *
 * @par Advertising
 *
 * Advertising consists of broadcasting at a regular interval a small amount of
 * data containing valuable information about the device. These packets may be
 * scanned by peer devices listening on BLE advertising channels.
 *
 * Scanners may also request additional information from a device advertising by
 * sending a scan request. If the broadcaster accepts scan requests, it can reply
 * with a scan response packet containing additional information.
 *
 * Advertising parameters are updated using setAdvertisingParams(). The main
 * advertising payload is updated using setAdvertisingPayload(), and the scan response
 * is updated using setAdvertisingScanResponse(). If the advertising is already
 * updated, the data will take effect from the next advertising event.
 *
 * To create a valid advertising payload and scan response, you may use
 * AdvertisingDataBuilder. You must first allocate memory and create an mbed::Span and
 * pass that into the AdvertisingDataBuilder, which will only be able to add as much
 * data as fits in the provided buffer. The builder accepts any size of the buffer,
 * but for the created data to be usable, it must be smaller than the maximum data
 * length returned from getMaxAdvertisingDataLength().
 *
 * Another option is to use AdvertisingDataSimpleBuilder, which allocates memory
 * on the stack and offers a fluent interface at the expense of a reduced set of
 * APIs and error management options.
 *
 * @note Prior to Bluetooth 5, advertising and scanning payload size were limited
 * to LEGACY_ADVERTISING_MAX_SIZE. It changed with Bluetooth 5, and now the maximum
 * size of data that can be advertised depends on the controller. If  you wish
 * to be compatible with older devices, you may wish to advertise with the
 * LEGACY_ADVERTISING_HANDLE. This uses payloads no larger than LEGACY_ADVERTISING_MAX_SIZE
 * with that advertising set.
 *
 * @par Extended advertising
 *
 * Extended advertising allows for a wider choice of options than legacy advertising.
 * You can send bigger payloads and use different PHYs. This allows for bigger throughput
 * or longer range.
 *
 * Extended advertising may be split across many packets and takes place on both the
 * regular advertising channels and the rest of the 37 channels normally used by
 * connected devices.
 *
 * The 3 channels used in legacy advertising are called primary advertisement channels.
 * The remaining 37 channels are used for secondary advertising. Unlike sending data
 * during a connection, this allows the device to broadcast data to multiple devices.
 *
 * The advertising starts on the primary channels (which you may select) and continues
 * on the secondary channels as indicated in the packet sent on the primary channel.
 * This way, the advertising can send large payloads without saturating the advertising
 * channels. Primary channels are limited to 1M and coded PHYs, but secondary channels
 * may use the increased throughput 2M PHY.
 *
 * @par Periodic advertising
 *
 * Similarly, you can use periodic advertising to transfer regular data to multiple
 * devices.
 *
 * The advertiser uses primary channels to advertise the information needed to
 * listen to the periodic advertisements on secondary channels. This sync information
 * will be used by the scanner who can now optimize for power consumption and only
 * listen for the periodic advertisements at specified times.
 *
 * Like extended advertising, periodic advertising offers extra PHY options of 2M
 * and coded. The payload may be updated at any time and will be updated on the next
 * advertisement event when the periodic advertising is active.
 *
 * @par Advertising sets
 *
 * Advertisers may advertise multiple payloads at the same time. The configuration
 * and identification of these is done through advertising sets. Use a handle
 * obtained from createAvertisingSet() for advertising operations. After ending
 * all advertising operations, remove the handle from the system using
 * destroyAdvertisingHandle().
 *
 * Extended advertising and periodic advertising is an optional feature, and not all
 * devices support it. Some will only be able to see the now-called legacy advertising.
 *
 * Legacy advertising is available through a special handle, LEGACY_ADVERTISING_HANDLE.
 * This handle is always available, doesn't need to be created and can't be
 * destroyed.
 *
 * There is a limited number of advertising sets available because they require support
 * from the controller. Their availability is dynamic and may be queried at any time
 * using getMaxAdvertisingSetNumber(). Advertising sets take up resources even if
 * they are not actively advertising right now, so it's important to destroy the set
 * when you're done with it (or reuse it in the next advertisement).
 *
 * Periodic advertising and extended advertising share the same set but not the same
 * data. Extended advertising carries out periodic advertising synchronization
 * information. Therefore, to let other devices be aware that your device
 * exposes periodic advertising, you should start extended advertising of the set.
 * Subsequently, you may disable extended advertising, and the periodic advertising
 * will continue. If you start periodic advertising while extended advertising is
 * inactive, periodic advertising won't start until you start extended advertising
 * at a later time.
 *
 * @par Privacy
 *
 * Privacy is a feature that allows a device to avoid being tracked by other
 * (untrusted) devices. The device achieves it by periodically generating a
 * new random address. The random address may be a resolvable random address,
 * enabling trusted devices to recognize it as belonging to the same
 * device. These trusted devices receive an Identity Resolution Key (IRK)
 * during pairing. This is handled by the SecurityManager and relies on the
 * other device accepting and storing the IRK.
 *
 * You need to enable privacy by calling enablePrivacy() after having
 * initialized the SecurityManager because privacy requires SecurityManager
 * to handle IRKs. The behavior of privacy enabled devices is set by
 * using setCentralPrivacyConfiguration(), which specifies what the device
 * should be with devices using random addresses. Random addresses
 * generated by privacy enabled devices can be of two types: resolvable
 * (by devices who have the IRK) and unresolvable. Unresolvable addresses
 * can't be used for connecting and connectable advertising. Therefore, a
 * resolvable one will be used for these regardless of the privacy
 * configuration.
 *
 * @par Scanning
 *
 * Scanning consists of listening for peer advertising packets. From a scan, a
 * device can identify devices available in its environment.
 *
 * If the device scans actively, then it will send scan request to scannable
 * advertisers and collect their scan responses.
 *
 * Scanning is done by creating ScanParameters and applying them with
 * setScanParameters(). One configured, you may call startScan().
 *
 * When a scanning device receives an advertising packet, it will call
 * onAdvertisingReport() in the registered event handler. A whitelist may be used
 * to limit the advertising reports by setting the correct policy in the scan
 * parameters.
 *
 * @par Connection event handling
 *
 * A peer may connect device advertising connectable packets. The advertising
 * procedure ends as soon as the device is connected. If an advertising timeout
 * has been set in the advertising parameters then onAdvertisingEnd will be called
 * in the registered eventHandler when it runs out.
 *
 * A device accepting a connection request from a peer is named a peripheral,
 * and the device initiating the connection is named a central.
 *
 * Connection is initiated by central devices. A call to connect() will result in
 * the device scanning on any PHYs set in ConectionParamters passed in.
 *
 * Peripheral and central receive a connection event when the connection is
 * effective. If successful will result in a call to onConnectionComplete in the
 * EventHandler registered with the Gap.
 *
 * It the connection attempt fails it will result in onConnectionComplete called
 * on the central device with the event carrying the error flag.
 *
 * @par Changing the PHYsical transport of a connection
 *
 * Once a connection has been established, it is possible to change the physical
 * transport used between the local and the distant device. Changing the transport
 * can either increase the bandwidth or increase the communication range.
 * An increased bandwidth equals a better power consumption but also a loss in
 * sensibility and therefore a degraded range.
 *
 * Symmetrically an increased range means a lowered bandwidth and a degraded power
 * consumption.
 *
 * Applications can change the PHY used by calling the function setPhy. Once the
 * update has been made the result is forwarded to the application by calling the
 * function onPhyUpdateComplete of the event handler registered.
 *
 * @par disconnection
 *
 * The application code initiates a disconnection when it calls the
 * disconnect(Handle_t, DisconnectionReason_t) function.
 *
 * Disconnection may also be initiated by the remote peer or the local
 * controller/stack. To catch all disconnection events, application code may
 * set up an handler taking care of disconnection events by calling
 * onDisconnection().
 *
 * @par Modulation Schemes
 *
 * When supported by the host and controller you can select different modulation
 * schemes (@see BLUETOOTH SPECIFICATION Version 5.0 | Vol 1, Part A - 1.2):
 * - LE 1M PHY
 * - LE 2M PHY
 * - LE coded PHY
 *
 * You may set preferred PHYs (separately for RX and TX) using setPreferredPhys().
 * You may also set the currently used PHYs on a selected connection using setPhy().
 * Both of these settings are only advisory and the controller is allowed to make
 * its own decision on the best PHY to use based on your request, the peer's
 * supported features and the connection's physical conditions.
 *
 * You may query the currently used PHY using readPhy() which will return the
 * result through a call to the registered event handler. You may register the
 * handler with setEventHandler(). The events inform about the currently used
 * PHY and of any changes to PHYs which may be triggered autonomously by the
 * controller or by the peer.
 */
class Gap {
public:
    /**
     * Definition of the general handler of Gap related events.
     */
    struct EventHandler {
        /**
         * Called when an advertising device receive a scan response.
         *
         * @param event Scan request event.
         *
         * @version: 5+.
         *
         * @see AdvertisingParameters::setScanRequestNotification().
         */
        virtual void onScanRequestReceived(const ScanRequestEvent &event)
        {
        }

        /**
         * Called when advertising starts.
         *
         * @param event Advertising start event.
         *
         * @see startAdvertising()
         */
        virtual void onAdvertisingStart(const AdvertisingStartEvent &event)
        {
        }

        /**
         * Called when advertising ends.
         *
         * Advertising ends when the process timeout or if it is stopped by the
         * application or if the local device accepts a connection request.
         *
         * @param event Advertising end event.
         *
         * @see startAdvertising()
         * @see stopAdvertising()
         * @see onConnectionComplete()
         */
        virtual void onAdvertisingEnd(const AdvertisingEndEvent &event)
        {
        }

        /**
         * Called when a scanner receives an advertising or a scan response packet.
         *
         * @param event Advertising report.
         *
         * @see startScan()
         */
        virtual void onAdvertisingReport(const AdvertisingReportEvent &event)
        {
        }

        /**
         * Called when scan times out.
         *
         * @param event Associated event.
         *
         * @see startScan()
         */
        virtual void onScanTimeout(const ScanTimeoutEvent &event)
        {
        }

        /**
         * Called when first advertising packet in periodic advertising is received.
         *
         * @param event Periodic advertising sync event.
         *
         * @version: 5+.
         *
         * @see createSync()
         */
        virtual void onPeriodicAdvertisingSyncEstablished(
            const PeriodicAdvertisingSyncEstablishedEvent &event
        )
        {
        }

        /**
         * Called when a periodic advertising packet is received.
         *
         * @param event Periodic advertisement event.
         *
         * @version: 5+.
         *
         * @see createSync()
         */
        virtual void onPeriodicAdvertisingReport(
            const PeriodicAdvertisingReportEvent &event
        )
        {
        }

        /**
         * Called when a periodic advertising sync has been lost.
         *
         * @param event Details of the event.
         *
         * @version: 5+.
         *
         * @see createSync()
         */
        virtual void onPeriodicAdvertisingSyncLoss(
            const PeriodicAdvertisingSyncLoss &event
        )
        {
        }

        /**
         * Called when connection attempt ends or an advertising device has been
         * connected.
         *
         * @see startAdvertising()
         * @see connect()
         *
         * @param event Connection event.
         */
        virtual void onConnectionComplete(const ConnectionCompleteEvent &event)
        {
        }

        /**
         * Called when the peer request connection parameters updates.
         *
         * Application must accept the update with acceptConnectionParametersUpdate()
         * or reject it with rejectConnectionParametersUpdate().
         *
         * @param event The connection parameters requested by the peer.
         *
         * @version 4.1+.
         *
         * @note This event is not generated if connection parameters update
         * is managed by the middleware.
         *
         * @see manageConnectionParametersUpdateRequest()
         * @see acceptConnectionParametersUpdate()
         * @see rejectConnectionParametersUpdate()
         */
        virtual void onUpdateConnectionParametersRequest(
            const UpdateConnectionParametersRequestEvent &event
        )
        {
        }

        /**
         * Called when connection parameters have been updated.
         *
         * @param event The new connection parameters.
         *
         * @see updateConnectionParameters()
         * @see acceptConnectionParametersUpdate()
         */
        virtual void onConnectionParametersUpdateComplete(
            const ConnectionParametersUpdateCompleteEvent &event
        )
        {
        }

        /**
         * Called when a connection has been disconnected.
         *
         * @param event Details of the event.
         *
         * @see disconnect()
         */
        virtual void onDisconnectionComplete(const DisconnectionCompleteEvent &event)
        {
        }

        /**
         * Function invoked when the current transmitter and receiver PHY have
         * been read for a given connection.
         *
         * @param status Status of the operation: BLE_ERROR_NONE in case of
         * success or an appropriate error code.
         *
         * @param connectionHandle: The handle of the connection for which the
         * PHYs have been read.
         *
         * @param txPhy PHY used by the transmitter.
         *
         * @param rxPhy PHY used by the receiver.
         *
         * @see readPhy().
         *
         * @version: 5+.
         */
        virtual void onReadPhy(
            ble_error_t status,
            connection_handle_t connectionHandle,
            phy_t txPhy,
            phy_t rxPhy
        )
        {
        }

        /**
         * Function invoked when the update process of the PHY has been completed.
         *
         * The process can be initiated by a call to the function setPhy, the
         * local bluetooth subsystem or the peer.
         *
         * @param status Status of the operation: BLE_ERROR_NONE in case of
         * success or an appropriate error code.
         *
         * @param connectionHandle: The handle of the connection on which the
         * operation was made.
         *
         * @param txPhy PHY used by the transmitter.
         *
         * @param rxPhy PHY used by the receiver.
         *
         * @note Success doesn't mean the PHY has been updated it means both
         * ends have negotiated the best PHY according to their configuration and
         * capabilities. The PHY currently used are present in the txPhy and
         * rxPhy parameters.
         *
         * @see setPhy()
         *
         * @version: 5+.
         */
        virtual void onPhyUpdateComplete(
            ble_error_t status,
            connection_handle_t connectionHandle,
            phy_t txPhy,
            phy_t rxPhy
        )
        {
        }

        /**
         * Function invoked when the connections changes the maximum number of octets
         * that can be sent or received by the controller in a single packet. A single
         * L2CAP packet can be fragmented across many such packets.
         *
         * @note This only triggers if controller supports data length extension and
         * negotiated data length is longer than the default 23.
         *
         * @param connectionHandle The handle of the connection that changed the size.
         * @param txSize Number of octets we can send on this connection in a single packet.
         * @param rxSize Number of octets we can receive on this connection in a single packet.
         */
        virtual void onDataLengthChange(
            connection_handle_t connectionHandle,
            uint16_t txSize,
            uint16_t rxSize
        )
        {
        }

        /**
         * Function invoked when the privacy subsystem has been enabled and is
         * ready to be used.
         */
        virtual void onPrivacyEnabled()
        {
        }
    protected:
        /**
         * Prevent polymorphic deletion and avoid unnecessary virtual destructor
         * as the Gap class will never delete the instance it contains.
         */
        ~EventHandler() = default;
    };

    /**
     * Preferred connection parameter display in Generic Access Service.
     */
    typedef struct {
        /**
         * Minimum interval between two connection events allowed for a
         * connection.
         *
         * It shall be less than or equal to maxConnectionInterval. This value,
         * in units of 1.25ms, is included in the range [0x0006 : 0x0C80].
         */
        uint16_t minConnectionInterval;

        /**
         * Maximum interval between two connection events allowed for a
         * connection.
         *
         * It shall be greater than or equal to minConnectionInterval. This
         * value is in unit of 1.25ms and is in the range [0x0006 : 0x0C80].
         */
        uint16_t maxConnectionInterval;

        /**
         * Number of connection events the slave can drop if it has nothing to
         * communicate to the master.
         *
         * This value shall be in the range [0x0000 : 0x01F3].
         */
        uint16_t slaveLatency;

        /**
         * Link supervision timeout for the connection.
         *
         * Time after which the connection is considered lost if the device
         * didn't receive a packet from its peer.
         *
         * It is larger than:
         *        (1 + slaveLatency) * maxConnectionInterval * 2
         *
         * This value is in the range [0x000A : 0x0C80] and is in unit of
         * 10 ms.
         *
         * @note maxConnectionInterval is in ms in the formulae above.
         */
        uint16_t connectionSupervisionTimeout;
    } PreferredConnectionParams_t;

    /**
     * Assign the event handler implementation that will be used by the gap
     * module to signal events back to the application.
     *
     * @param handler Application implementation of an EventHandler.
     *
     * @note Multiple discrete EventHandler instances may be used by adding them
     * to a ChainableGapEventHandler and then setting the chain as the primary
     * Gap EventHandler using this function.
     *
     * @see ChainableGapEventHandler
     */
    void setEventHandler(EventHandler *handler)
    {
        _handler = handler;
    }

    EventHandler *getEventHandler()
    {
        return _handler;
    }

private:
    EventHandler *_handler = nullptr;
};

/**
 * @}
 * @}
 */

} // namespace ble

/** @deprecated Use the namespaced ble::Gap instead of the global Gap. */
using ble::Gap;

#endif // BLE_GAP_GAP_H
